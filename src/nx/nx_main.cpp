// nx_main.cpp -- Nintendo Switch entry point. Initializes libnx services,
// sets up the game directory, then hands control to the game's WinMain
// (src/win32/win_main.cpp), which never returns.
#include <switch.h>

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
