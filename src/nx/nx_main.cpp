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
static int s_logState;          // 0 idle, 1 the thread is writing, 2 crashed: the crash handler writes
static bool s_logThreadDone;    // the thread finished its last write after a crash

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
        int idle = 0;
        if (!__atomic_compare_exchange_n(&s_logState, &idle, 1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
            mutexUnlock(&s_logLock);   // crashed: the crash handler writes the rest
            return;
        }
        mutexUnlock(&s_logLock);
        ssize_t written = write(s_logFd, s_logRing + at, chunk);   // outside the lock
        mutexLock(&s_logLock);
        s_logTail += written > 0 ? (size_t)written : chunk;
        condvarWakeAll(&s_logHasRoom);
        mutexUnlock(&s_logLock);
        int writing = 1;
        if (!__atomic_compare_exchange_n(&s_logState, &writing, 0, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
            __atomic_store_n(&s_logThreadDone, true, __ATOMIC_SEQ_CST);
            return;
        }
    }
}

// Exit: everything printed reaches the file.
static void nxExitStep(const char *step);

static void nxLogShutdown(void)
{
    nxExitStep("atexit: log shutdown, fflush(stdout/stderr)");
    fflush(stdout);
    fflush(stderr);
    nxExitStep("atexit: log shutdown, stopping the log thread");
    mutexLock(&s_logLock);
    s_logStop = true;
    condvarWakeAll(&s_logHasData);
    mutexUnlock(&s_logLock);
    threadWaitForExit(&s_logThread);
    threadClose(&s_logThread);
}

// Crash: whatever the ring holds, without the lock (its holder may be the
// thread that faulted), then the report goes straight to the file.
//
// The writer thread may be in the middle of a write() of the same bytes. The
// drain used to write them again, then the thread moved the tail past the head
// and wrote the whole ring over and over: a crash left a log of tens of MB,
// the same lines four or five times. Now the first drain stops the thread and
// waits (up to a second) for a write it has in progress to finish.
static void nxLogDrainUnlocked(void)
{
    if (s_logFd < 0)
        return;
    if (__atomic_exchange_n(&s_logState, 2, __ATOMIC_SEQ_CST) == 1) {
        for (int ms = 0; ms < 1000 && !__atomic_load_n(&s_logThreadDone, __ATOMIC_SEQ_CST); ++ms)
            svcSleepThread(1000000);
    }
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

// The NRO this process was started from (argv[0] from the homebrew loader or a
// forwarder), for chain-loading its sibling.
static char s_nroPath[512];

// The SP <-> MP handoff (retail: BlackOps.exe / BlackOpsMP.exe through Steam).
// Asks the homebrew loader to start <nroName> from this NRO's folder once this
// one exits; the caller then quits normally (Sys_Quit -> exit). A loader that
// cannot chain-load (envHasNextLoad false) just returns to its menu.
// The installed title (a forwarder) whose NACP name is <name>, or 0. Logs every
// installed title with "kisak" in its name, so a renamed forwarder shows up.
static u64 nxFindInstalledTitle(const char *name)
{
    if (R_FAILED(nsInitialize()))
        return 0;
    NsApplicationControlData *control = (NsApplicationControlData *)malloc(sizeof(NsApplicationControlData));
    NsApplicationRecord records[32];
    s32 offset = 0, count = 0;
    u64 found = 0;
    while (control && !found && R_SUCCEEDED(nsListApplicationRecord(records, 32, offset, &count)) && count > 0) {
        for (s32 i = 0; i < count && !found; ++i) {
            u64 size = 0;
            NacpLanguageEntry *entry = nullptr;
            if (R_FAILED(nsGetApplicationControlData(NsApplicationControlSource_Storage, records[i].application_id,
                                                     control, sizeof(*control), &size))
                || R_FAILED(nacpGetLanguageEntry(&control->nacp, &entry)) || !entry)
                continue;
            char lower[sizeof(entry->name)];
            size_t n = 0;
            for (; n + 1 < sizeof(lower) && entry->name[n]; ++n)
                lower[n] = (char)tolower((unsigned char)entry->name[n]);
            lower[n] = '\0';
            if (strstr(lower, "kisak"))
                printf("[nx] installed title %016lx '%s'\n", records[i].application_id, entry->name);
            if (!strcasecmp(entry->name, name))
                found = records[i].application_id;
        }
        offset += count;
    }
    free(control);
    nsExit();
    return found;
}

extern "C" bool NX_ChainLoadSibling(const char *nroName)
{
    char path[512];
    const char *slash = strrchr(s_nroPath, '/');
    if (s_nroPath[0] && slash)
        snprintf(path, sizeof(path), "%.*s/%s", (int)(slash - s_nroPath), s_nroPath, nroName);
    else
        snprintf(path, sizeof(path), "%s/%s", NX_GAME_DIR, nroName);

    // The loader (hbmenu, sphaira, a forwarder) starts the NRO once this one
    // exits, as for any homebrew. That needs a clean exit (NX_PrepareExit):
    // before it, KisakBlack's threads outlived the exit and the next NRO
    // crashed (Atmosphere report: the render thread running unmapped code).
    if (envHasNextLoad()) {
        char args[600];
        snprintf(args, sizeof(args), "\"%s\"", path);
        Result rc = envSetNextLoad(path, args);
        printf("[nx] next load: %s (rc 0x%x)\n", path, rc);
        if (R_SUCCEEDED(rc))
            return true;
    }

    // A loader that cannot chain-load: when running as an application, launch
    // the sibling's installed forwarder title instead (a fresh process after
    // this one exits).
    const AppletType appletType = appletGetAppletType();
    if (appletType == AppletType_Application || appletType == AppletType_SystemApplication) {
        char title[64];
        snprintf(title, sizeof(title), "%s", nroName);
        if (char *dot = strrchr(title, '.'))
            *dot = '\0';
        const u64 titleId = nxFindInstalledTitle(title);
        if (titleId) {
            Result rc = appletRequestLaunchApplication(titleId, nullptr);
            printf("[nx] launch title %016lx '%s' after exit (rc 0x%x)\n", titleId, title, rc);
            if (R_SUCCEEDED(rc))
                return true;
        }
    }
    printf("[nx] cannot start %s from this loader; quitting\n", path);
    return true;
}

// ---------------------------------------------------------------------------
// Clean exit back to the homebrew loader. The loader runs the next NRO (the
// SP <-> MP button, or just its menu) in this same process, so nothing of this
// one may keep running or keep memory the next one will use. Sys_Quit calls
// NX_PrepareExit right before exit():
//  - the mixer thread stops and audout closes (SD_Shutdown; idempotent);
//  - every engine thread is paused for good and its stack freed
//    (NX_FreezeEngineThreads, nx_wincompat.cpp);
// and as the very last atexit step the GPU service is closed: mesa's EGL took
// a reference to libnx's nv service, whose transfer memory comes out of the
// heap the next NRO gets, and eglTerminate never runs (the GL context belongs
// to the render thread, now frozen). Closing the session frees every GPU
// allocation in the kernel. The log thread and sockets close in their own
// atexit handlers.
// ---------------------------------------------------------------------------
void SD_Shutdown();                                 // nx_snd.cpp
extern "C" int NX_FreezeEngineThreads(void);        // nx_wincompat.cpp
extern "C" bool NX_GlRelease(unsigned int timeoutMs); // nx_d3d9_null.cpp
static bool s_exitPrepared;
static bool s_glReleased;

// Exit watchdog. The first clean-exit build froze in exit() after the threads
// were frozen, with the log thread already gone. Each step below records its
// name; if exit has not reached userAppExit (after every atexit handler and
// destructor) within 8 s, the watchdog writes the step straight to the log
// file and ends the process -- the title closes instead of hanging.
static const char *volatile s_exitStep = "not exiting";
static Thread s_exitWatchdog;
static UEvent s_exitDone;
static bool s_exitWatchdogRunning;

static void nxExitStep(const char *step)
{
    s_exitStep = step;
}

static void nxExitWatchdog(void *)
{
    if (R_SUCCEEDED(waitSingle(waiterForUEvent(&s_exitDone), 8000000000ull)))
        return;
    char line[160];
    int n = snprintf(line, sizeof(line), "[nx] exit stalled for 8 s at: %s; ending the process\n", (const char *)s_exitStep);
    if (s_logFd >= 0 && n > 0)
        write(s_logFd, line, (size_t)n);
    svcExitProcess();
}

// Last step before the loader takes the heap back to load the next NRO into
// it. hbl aborted with 0xD401 (invalid current memory state) on the first
// exit that got this far: the heap still had the guard pages
// (nx_wincompat.cpp: a Perm_None page after every VirtualAlloc block and
// large malloc, up to 1500, never freed at exit). Make every heap page plain
// read-write again and clear uncached marks; log what was fixed and anything
// still borrowed, IPC- or device-mapped (a service still holds it).
static void nxExitWrite(const char *line, int n)
{
    if (s_logFd >= 0 && n > 0)
        write(s_logFd, line, (size_t)n);
}

static void nxRestoreHeapForNextLoad(void)
{
    char line[200];
    int permFixed = 0, attrFixed = 0, held = 0;
    u64 permBytes = 0;
    MemoryInfo info;
    u32 pageInfo;
    for (u64 addr = 0; R_SUCCEEDED(svcQueryMemory(&info, &pageInfo, addr)) && info.size; addr = info.addr + info.size) {
        if (info.addr + info.size <= addr)
            break;   // wrapped: end of the address space
        if (info.type != MemType_Heap)
            continue;
        if (info.perm != Perm_Rw && R_SUCCEEDED(svcSetMemoryPermission((void *)info.addr, info.size, Perm_Rw))) {
            ++permFixed;
            permBytes += info.size;
        }
        if ((info.attr & MemAttr_IsUncached)
            && R_SUCCEEDED(svcSetMemoryAttribute((void *)info.addr, info.size, MemAttr_IsUncached, 0)))
            ++attrFixed;
        if (info.attr & (MemAttr_IsBorrowed | MemAttr_IsIpcMapped | MemAttr_IsDeviceMapped)) {
            if (held++ < 20)
                nxExitWrite(line, snprintf(line, sizeof(line),
                    "[nx] exit: heap %#lx +%#lx still held: attr %#x ipc %u device %u\n",
                    info.addr, info.size, info.attr, info.ipc_refcount, info.device_refcount));
        }
    }
    nxExitWrite(line, snprintf(line, sizeof(line),
        "[nx] exit: heap restored for the next NRO: %d ranges made read-write (%lu KB), %d uncached cleared, %d still held\n",
        permFixed, (unsigned long)(permBytes >> 10), attrFixed, held));
}

extern "C" void userAppExit(void)
{
    if (!s_exitWatchdogRunning)
        return;
    nxExitStep("userAppExit: restoring the heap");
    nxRestoreHeapForNextLoad();
    nxExitStep("userAppExit");
    ueventSignal(&s_exitDone);
    threadWaitForExit(&s_exitWatchdog);
    threadClose(&s_exitWatchdog);
    s_exitWatchdogRunning = false;
}

extern "C" void NX_PrepareExit(void)
{
    if (s_exitPrepared)
        return;
    ueventCreate(&s_exitDone, false);
    if (R_SUCCEEDED(threadCreate(&s_exitWatchdog, nxExitWatchdog, nullptr, nullptr, 0x4000, 0x2B, -2))
        && R_SUCCEEDED(threadStart(&s_exitWatchdog)))
        s_exitWatchdogRunning = true;
    nxExitStep("SD_Shutdown");
    SD_Shutdown();
    nxExitStep("releasing EGL on the render thread");
    s_glReleased = NX_GlRelease(2000);
    nxExitStep("freezing engine threads");
    const int frozen = NX_FreezeEngineThreads();
    s_exitPrepared = true;
    printf("[nx] exit: audio closed, EGL %s, %d engine threads frozen\n",
           s_glReleased ? "released" : "NOT released (render thread did not answer)", frozen);
    nxExitStep("exit(): atexit handlers registered after main's (mesa, libraries)");
}

static void nxReleaseGpuAtExit(void)
{
    if (!s_exitPrepared)
        return;
    // With EGL still up the nv session cannot give its transfer memory back and
    // nvExit would wait forever; skip it then (the next NRO may still crash).
    if (s_glReleased) {
        nxExitStep("atexit: nvExit");
        for (int i = 0; i < 16; ++i)   // any references mesa left; extra calls do nothing
            nvExit();
    }
    nxExitStep("after atexit: C++ destructors, libc cleanup");
}

// Called from Win_GetEvent on the main thread every frame. Nothing read the
// applet's messages before, so closing the title from HOME went unanswered:
// with appletLockExit held the system waited, then killed the process (shown
// as a crash), and settings were never written. True once the system asks the
// title to close; the caller then runs the normal quit, which exits through
// exit() -> atexit appletUnlockExit. Other messages get libnx's handling.
extern "C" bool NX_PumpAppletMessages(void)
{
    static bool s_exitRequested;
    u32 msg = 0;
    while (!s_exitRequested && R_SUCCEEDED(appletGetMessage(&msg))) {
        if (msg == AppletMessage_ExitRequest) {
            printf("[nx] the system asked the title to close (HOME); quitting\n");
            s_exitRequested = true;
        } else {
            appletProcessMessage(msg);
        }
    }
    return s_exitRequested;
}

int main(int argc, char **argv)
{
    // Registered first, so it runs last of the atexit handlers.
    atexit(nxReleaseGpuAtExit);

    if (argc > 0 && argv && argv[0])
        snprintf(s_nroPath, sizeof(s_nroPath), "%s", argv[0]);

    // Keep the applet alive while the game runs its own main loop.
    appletLockExit();

    socketInitializeDefault();

    atexit([]{ nxExitStep("atexit: socketExit (2)"); socketExit(); });
    mkdir(NX_GAME_DIR, 0777);
    socketInitializeDefault();
    atexit([]{ nxExitStep("atexit: socketExit (1)"); socketExit(); });
    atexit([]{ nxExitStep("atexit: appletUnlockExit"); appletUnlockExit(); });
    chdir(NX_GAME_DIR);

    nxSetupLogging();
    NX_ClockInit();
    atexit([]{ nxExitStep("atexit: NX_ClockExit"); NX_ClockExit(); });

    nx_wincompat_init_main_thread();

    // Sys_Print writes every message twice: once via OutputDebugStringA and
    // once via Conbuf_AppendText. Both land in the same log here, so mute the
    // OutputDebugStringA copy (tlPrintf still uses it directly).
    extern int enable_OutputDebugString;
    enable_OutputDebugString = 0;

    printf("KisakBlack Switch port starting (dir: %s)\n", NX_GAME_DIR);
    printf("nro: %s (chain-load %s)\n", s_nroPath[0] ? s_nroPath : "<no argv[0]>",
           envHasNextLoad() ? "supported" : "not supported by this loader");
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
