// nx_main.cpp -- Nintendo Switch entry point. Initializes libnx services,
// sets up the game directory, then hands control to the game's WinMain
// (src/win32/win_main.cpp), which never returns.
#include <switch.h>
#include "nx_clock.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/iosupport.h>
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

    // The startup commands run in order, and a map command loads the level
    // there and then: a "+set" after it (cmdline.txt lines added below an
    // existing +devmap) only took effect after the textures had loaded --
    // r_picmip 2 was used for the frontend and then back to 3 for the map.
    // So the map command, with its map name, moves to the end.
    static const char *const mapCommands[] = { "+devmap ", "+map ", "+spdevmap ", "+spmap " };
    for (const char *cmd : mapCommands) {
        char *at = strstr(cmdline, cmd);
        if (!at)
            continue;
        char *name = at + strlen(cmd);
        while (*name == ' ')
            ++name;
        char *end = name;
        while (*end && *end != ' ')
            ++end;
        char moved[128];
        snprintf(moved, sizeof(moved), " %.*s", (int)(end - at), at);
        memmove(at, end, strlen(end) + 1);
        if (strlen(cmdline) + strlen(moved) + 1 < size)
            strcat(cmdline, moved);
        break;
    }
}

// Route stdout/stderr somewhere the user can read after a crash: nxlink if the
// title was launched from it, otherwise a log file on the SD card. Both are
// unbuffered so nothing is lost when the process aborts.
//
// Both streams open the file in append mode, after one truncating open. They
// used to share it through dup2, but each kept its own write position, so
// whichever wrote second overwrote the other -- and when one position ran
// past the end of the file, the gap came back as whatever the SD card held
// there: the stray binary in front of some lines, and a GL renderer string
// that never reached the log. Append mode makes newlib seek to the current
// end before every write, so the two can interleave but never overlap.
// ----- Asynchronous log -----
// Every printf used to be a write to the SD card on the thread that printed:
// the 600-frame summaries (~60 lines) made a 140-190 ms hitch each time. Now
// stdout and stderr are line-buffered FILEs whose output goes into a ring in
// memory, and a low-priority thread writes the ring to the log file. A full
// ring makes the printing thread wait, so nothing is lost. The crash handler
// writes out what the ring still holds before its own report (nxLogDrainUnlocked).
static int s_logFd = -1;
static char s_logRing[4 << 20];
static size_t s_logHead, s_logTail;   // write at head, the thread reads from tail
static Mutex s_logLock;
static CondVar s_logHasData, s_logHasRoom;
static Thread s_logThread;
static volatile bool s_logStop;

static ssize_t nxLogWrite(struct _reent *, void *, const char *data, size_t length)
{
    const int size = (int)length;
    if (size <= 0)
        return 0;
    mutexLock(&s_logLock);
    for (int done = 0; done < size;) {
        size_t used = s_logHead - s_logTail;
        if (used == sizeof(s_logRing)) {
            condvarWait(&s_logHasRoom, &s_logLock);
            continue;
        }
        size_t chunk = sizeof(s_logRing) - used;
        if (chunk > (size_t)(size - done))
            chunk = (size_t)(size - done);
        size_t at = s_logHead % sizeof(s_logRing);
        if (chunk > sizeof(s_logRing) - at)
            chunk = sizeof(s_logRing) - at;
        memcpy(s_logRing + at, data + done, chunk);
        s_logHead += chunk;
        done += (int)chunk;
    }
    condvarWakeOne(&s_logHasData);
    mutexUnlock(&s_logLock);
    return size;
}

static void nxLogThread(void *)
{
    for (;;) {
        mutexLock(&s_logLock);
        while (s_logHead == s_logTail && !s_logStop)
            condvarWait(&s_logHasData, &s_logLock);
        if (s_logHead == s_logTail && s_logStop) {
            mutexUnlock(&s_logLock);
            return;
        }
        size_t at = s_logTail % sizeof(s_logRing);
        size_t chunk = s_logHead - s_logTail;
        if (chunk > sizeof(s_logRing) - at)
            chunk = sizeof(s_logRing) - at;
        mutexUnlock(&s_logLock);
        ssize_t written = write(s_logFd, s_logRing + at, chunk);   // outside the lock
        mutexLock(&s_logLock);
        s_logTail += written > 0 ? (size_t)written : chunk;
        condvarWakeAll(&s_logHasRoom);
        mutexUnlock(&s_logLock);
    }
}

// Exit: everything printed reaches the file.
static void nxLogShutdown(void)
{
    fflush(stdout);
    fflush(stderr);
    mutexLock(&s_logLock);
    s_logStop = true;
    condvarWakeAll(&s_logHasData);
    mutexUnlock(&s_logLock);
    threadWaitForExit(&s_logThread);
    threadClose(&s_logThread);
}

// Crash: whatever the ring holds, without the lock (its holder may be the
// thread that faulted), then the report goes straight to the file.
static void nxLogDrainUnlocked(void)
{
    if (s_logFd < 0)
        return;
    while (s_logTail < s_logHead) {
        size_t at = s_logTail % sizeof(s_logRing);
        size_t chunk = s_logHead - s_logTail;
        if (chunk > sizeof(s_logRing) - at)
            chunk = sizeof(s_logRing) - at;
        ssize_t written = write(s_logFd, s_logRing + at, chunk);
        if (written <= 0)
            break;
        s_logTail += (size_t)written;
    }
}

static void nxSetupLogging(void)
{
    if (nxlinkStdio() >= 0) {
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);
        return;
    }
    s_logFd = open(NX_GAME_DIR "/kisakblack.log", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (s_logFd < 0)
        return;
    mutexInit(&s_logLock);
    condvarInit(&s_logHasData);
    condvarInit(&s_logHasRoom);
    if (R_FAILED(threadCreate(&s_logThread, nxLogThread, nullptr, nullptr, 0x10000, 0x3B, -2))
        || R_FAILED(threadStart(&s_logThread))) {
        // No thread: write as before, synchronously.
        dup2(s_logFd, fileno(stdout));
        dup2(s_logFd, fileno(stderr));
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);
        return;
    }
    // Every thread's stdout and stderr end on descriptors 1 and 2, whose
    // device this replaces -- the way libnx routes them to nxlink.
    static devoptab_t s_logDevice;
    s_logDevice.name = "kblog";
    s_logDevice.write_r = nxLogWrite;
    devoptab_list[STD_OUT] = &s_logDevice;
    devoptab_list[STD_ERR] = &s_logDevice;
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    atexit(nxLogShutdown);
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
// then ends the process.
//
// It does not hand the exception back. svcReturnFromException is not among
// the syscalls this process may make -- the title it runs under does not
// grant it -- so calling it raised a bad-SVC exception, which came straight
// back here, forever: the console froze and had to be forced off.
// svcExitProcess is always allowed, and a second entry, from another thread
// or from a fault in here, goes straight to it.
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
    if (n <= 0)
        return;
    // The asynchronous log first, so the lines before the crash come first.
    nxLogDrainUnlocked();
    write(s_logFd >= 0 ? s_logFd : fileno(stdout), buf, n < (int)sizeof(buf) ? n : (int)sizeof(buf) - 1);
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

static void nxCrashReport(ThreadExceptionDump *ctx);

// Every thread's exception runs on the one __nx_exception_stack, with its dump
// at the top. When a second thread faults while the first is reporting, its
// entry overwrites that dump and the report's frames, and the log stopped
// right after the header. So the first entry copies the dump out and reports
// from below a pad the second entry's small frame never reaches; the second
// waits for the report to finish before ending the process.
static ThreadExceptionDump s_crashDump;

extern "C" void __libnx_exception_handler(ThreadExceptionDump *ctx)
{
    static int s_entered;
    if (__atomic_fetch_add(&s_entered, 1, __ATOMIC_SEQ_CST))
    {
        svcSleepThread(3000000000ull);
        svcExitProcess();
    }
    s_crashDump = *ctx;
    volatile u8 pad[0x4000];
    pad[0] = 0;
    nxCrashReport(&s_crashDump);
    pad[1] = 0;
    svcExitProcess();
}

static __attribute__((noinline)) void nxCrashReport(ThreadExceptionDump *ctx)
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
    nxCrashWrite("[nx-crash] exiting\n");

    svcExitProcess();
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
    NX_ClockInit();
    atexit(NX_ClockExit);

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
    //
    // Then the lowest graphics settings, resolution aside. Com_StartupVariable
    // applies +set lines after the saved config and just before the renderer
    // starts, so textures load at the reduced size; cmdline.txt comes after
    // and can override any of them.
    //   picmip 2        color, normal and specular maps at 1/4 size. With
    //                   r_stream 0 (no high mips streamed in) 3 looked too
    //                   blurry; 1 brought ~200 ms driver stalls in game
    //                   (README-SWITCH, "picmip").
    //   sm_enable 0     no shadow maps (their passes are whole extra scenes)
    //   depthPrepass 0  no depth-only pass over the opaque world
    //   dof, distortion, flame, marks, brass: extra passes and draws
    //   nx_bloom 0      no bloom: ~10 extra passes, and it drew light glows
    //                   offset from their lights (README-SWITCH, known problems)
    static char cmdline[2048] =
        "allowdupe nodump"
        " +set r_picmip_manual 1 +set r_picmip 2 +set r_picmip_bump 2 +set r_picmip_spec 2"
        " +set r_texFilterAnisoMax 1 +set r_aaSamples 1"
        " +set sm_enable 0 +set r_depthPrepass 0"
        " +set r_dof_enable 0 +set r_distortion 0 +set r_flame_allowed 0 +set nx_bloom 0"
        " +set fx_marks 0 +set fx_marks_ents 0 +set cg_brass 0";
    // The game resolution by where the title starts: 960x540 handheld, where
    // the GPU (307-460 MHz) is the limit, 1280x720 docked. The present blit
    // scales it to the 1280x720 window. Chosen once, at boot: docking later
    // keeps it (changing it live is a vid_restart this port has not done).
    strcat(cmdline, appletGetOperationMode() == AppletOperationMode_Handheld
                        ? " +set r_mode 960x540" : " +set r_mode 1280x720");
    nxAppendCmdlineFile(cmdline, sizeof(cmdline));
    printf("command line: %s\n", cmdline);
    nx_set_command_line(cmdline);

    // WinMain runs the engine and never returns (infinite Com_Frame loop).
    int rc = WinMain((HINSTANCE)1, NULL, cmdline, 1);

    socketExit();
    appletUnlockExit();
    return rc;
}
