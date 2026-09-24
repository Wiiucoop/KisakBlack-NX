// nx_main.cpp -- Nintendo Switch entry point. Initializes libnx services,
// sets up the game directory, then hands control to the game's WinMain
// (src/win32/win_main.cpp), which never returns.
#include <switch.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <windows.h>

#define NX_GAME_DIR "sdmc:/switch/kisakblack"

extern "C" void nx_wincompat_init_main_thread(void);
extern "C" void nx_set_command_line(const char *cmdline);

static char s_installDir[512] = NX_GAME_DIR;

extern "C" const char *nx_get_install_dir(void)
{
    return s_installDir;
}

// Larger-than-default newlib heap; the engine allocates aggressively.
// (Only takes effect in applications with enough resource; under hbl the
// available memory is what it is.)
extern "C" u32 __nx_heap_size_mult; // not a real libnx symbol; kept for doc

static void nxAppendCmdlineFile(char *cmdline, size_t size)
{
    // sdmc:/switch/kisakblack/cmdline.txt lets the user tweak startup args
    // (e.g. "+set dedicated 2 +set net_ip 192.168.1.50") without rebuilding.
    FILE *f = fopen(NX_GAME_DIR "/cmdline.txt", "rb");
    if (!f) return;
    size_t len = strlen(cmdline);
    if (len + 2 < size) {
        cmdline[len++] = ' ';
        size_t n = fread(cmdline + len, 1, size - len - 1, f);
        cmdline[len + n] = 0;
        // strip CR/LF
        for (char *p = cmdline; *p; ++p)
            if (*p == '\r' || *p == '\n') *p = ' ';
    }
    fclose(f);
}

// Route stdout/stderr somewhere the user can read after a crash: nxlink if the
// title was launched from it, otherwise a log file on the SD card. Both are
// unbuffered so nothing is lost when the process aborts.
static void nxSetupLogging(void)
{
    if (nxlinkStdio() < 0) {
        freopen(NX_GAME_DIR "/kisakblack.log", "w", stdout);
        dup2(fileno(stdout), fileno(stderr));
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

extern "C" void nx_mem_status(uint64_t *total, uint64_t *avail); // nx_wincompat.cpp

static void nxReportMemory(void)
{
    uint64_t total = 0, freeHeap = 0;
    nx_mem_status(&total, &freeHeap);
    printf("memory: %llu MB total, %llu MB free heap\n",
           (unsigned long long)(total >> 20), (unsigned long long)(freeHeap >> 20));
    if (freeHeap < (700ull << 20)) {
        printf("WARNING: only %llu MB of heap -- this looks like APPLET mode.\n"
               "         KisakBlack needs application mode: launch hbmenu by holding R\n"
               "         while opening an installed game, then run KisakBlack from there.\n",
               (unsigned long long)(freeHeap >> 20));
    }
}

// ----- Crash reporting -----
// libnx runs __libnx_exception_handler on the stack below when a thread
// faults. It logs where, as offsets into the module -- the ELF is linked at 0,
// so they go straight to addr2line against the ELF from the same build -- and
// then hands the exception back unhandled, so Atmosphere still writes its own
// crash report.
//
// The build omits frame pointers (-O2), so there is no frame chain to walk.
// Instead the faulting thread's stack is scanned for words that point into the
// module's code: every live return address is among them, with some stale
// ones mixed in. Read them top down.
extern "C" {
alignas(16) u8 __nx_exception_stack[0x8000];
u64 __nx_exception_stack_size = sizeof(__nx_exception_stack);
}

// printf could deadlock on a stdout lock the faulting thread holds; format
// into a local buffer and write() it instead.
static void nxCrashWrite(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (n > 0)
        write(fileno(stdout), buf, n < (int)sizeof(buf) ? n : (int)sizeof(buf) - 1);
}

static const char *nxExceptionName(u32 desc)
{
    switch (desc) {
    case ThreadExceptionDesc_InstructionAbort: return "instruction abort";
    case ThreadExceptionDesc_MisalignedPC:     return "misaligned PC";
    case ThreadExceptionDesc_MisalignedSP:     return "misaligned SP";
    case ThreadExceptionDesc_SError:           return "SError";
    case ThreadExceptionDesc_BadSVC:           return "bad SVC";
    case ThreadExceptionDesc_Trap:             return "trap (incl. __builtin_trap / __debugbreak)";
    case ThreadExceptionDesc_Other:            return "data abort or other";
    default:                                   return "unknown";
    }
}

extern "C" void __libnx_exception_handler(ThreadExceptionDump *ctx)
{
    MemoryInfo text = {};
    u32 pageInfo;
    svcQueryMemory(&text, &pageInfo, (u64)&__libnx_exception_handler);
    const u64 textBase = text.addr, textEnd = text.addr + text.size;
    auto inText = [&](u64 a) { return a >= textBase && a < textEnd; };

    nxCrashWrite("\n[nx-crash] ===== CRASH: %s (error_desc 0x%x) =====\n",
                 nxExceptionName(ctx->error_desc), ctx->error_desc);
    nxCrashWrite("[nx-crash] module text 0x%llx..0x%llx; offsets below are KisakBlack.elf addresses\n",
                 (unsigned long long)textBase, (unsigned long long)textEnd);
    const u64 pc = ctx->pc.x, lr = ctx->lr.x;
    if (inText(pc))
        nxCrashWrite("[nx-crash] pc  elf+0x%llx\n", (unsigned long long)(pc - textBase));
    else
        nxCrashWrite("[nx-crash] pc  0x%llx (outside the module)\n", (unsigned long long)pc);
    if (inText(lr))
        nxCrashWrite("[nx-crash] lr  elf+0x%llx\n", (unsigned long long)(lr - textBase));
    else
        nxCrashWrite("[nx-crash] lr  0x%llx (outside the module)\n", (unsigned long long)lr);
    nxCrashWrite("[nx-crash] far 0x%llx  esr 0x%x  sp 0x%llx\n", (unsigned long long)ctx->far.x, ctx->esr,
                 (unsigned long long)ctx->sp.x);
    for (int i = 0; i < 29; i += 4) {
        char line[160];
        int n = 0;
        for (int j = i; j < i + 4 && j < 29; ++j)
            n += snprintf(line + n, sizeof(line) - n, " x%-2d %016llx", j, (unsigned long long)ctx->cpu_gprs[j].x);
        nxCrashWrite("[nx-crash] %s\n", line);
    }

    MemoryInfo stack = {};
    svcQueryMemory(&stack, &pageInfo, ctx->sp.x);
    const u64 *word = (const u64 *)ctx->sp.x;
    const u64 *stackEnd = (const u64 *)(stack.addr + stack.size);
    int found = 0;
    nxCrashWrite("[nx-crash] code addresses on the stack, innermost first:\n");
    for (int i = 0; i < 4096 && word + i < stackEnd && found < 32; ++i) {
        if (inText(word[i])) {
            nxCrashWrite("[nx-crash]   sp+0x%-5x elf+0x%llx\n", i * 8, (unsigned long long)(word[i] - textBase));
            ++found;
        }
    }
    nxCrashWrite("[nx-crash] resolve with: aarch64-none-elf-addr2line -f -C -e build-nx/KisakBlack.elf <elf+ offsets>\n");

    svcReturnFromException(MAKERESULT(Module_Kernel, KernelError_UnhandledUserInterrupt));
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    // Keep the applet alive while the game runs its own main loop.
    appletLockExit();

    socketInitializeDefault();

    atexit(socketExit);
    mkdir(NX_GAME_DIR, 0777);
    socketInitializeDefault();
    atexit(socketExit);
    atexit([]{ appletUnlockExit(); });
    chdir(NX_GAME_DIR);

    nxSetupLogging();

    nx_wincompat_init_main_thread();

    // Sys_Print writes every message twice: once via OutputDebugStringA and
    // once via Conbuf_AppendText. Both land in the same log here, so mute the
    // OutputDebugStringA copy (tlPrintf still uses it directly).
    extern int enable_OutputDebugString;
    enable_OutputDebugString = 0;

    printf("KisakBlack Switch port starting (dir: %s)\n", NX_GAME_DIR);
    printf("build: " __DATE__ " " __TIME__ " (KBZ loader rev6-loadcount)\n");
    nxReportMemory();

    // "allowdupe" (must be a prefix) skips the duplicate-instance semaphore
    // file; "nodump" skips the minidump handler.
    static char cmdline[2048] = "allowdupe nodump";
    nxAppendCmdlineFile(cmdline, sizeof(cmdline));
    nx_set_command_line(cmdline);

    // WinMain runs the engine and never returns (infinite Com_Frame loop).
    int rc = WinMain((HINSTANCE)1, NULL, cmdline, 1);

    socketExit();
    appletUnlockExit();
    return rc;
}
