// nx_wincompat.cpp -- Win32 API implementation over libnx/newlib for the
// Switch build. Companion to src/nx/compat/windows.h.
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <ShlObj.h>
#include <io.h>
#include <direct.h>

#include <switch.h>

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h> // memalign
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

// ===========================================================================
// path normalization ('\' separators and case come from Windows-era code)
// ===========================================================================
extern "C" void nx_normalize_path(const char *in, char *out, size_t outSize)
{
    size_t j = 0;
    if (!in) { if (outSize) out[0] = 0; return; }
    for (size_t i = 0; in[i] && j + 1 < outSize; ++i) {
        char c = in[i];
        if (c == '\\') c = '/';
        // collapse duplicate slashes (but keep "sdmc:/" intact)
        if (c == '/' && j > 0 && out[j-1] == '/') continue;
        out[j++] = c;
    }
    out[j] = 0;
}

static void nx_norm(const char *in, char *buf) { nx_normalize_path(in, buf, 1024); }

// Game code opens files with backslash-separated paths everywhere; the whole
// binary is linked with -Wl,--wrap=fopen/remove/rename so every call goes
// through these normalizing wrappers (macro interception would break under
// libstdc++'s <cstdio>, which #undefs stdio macros).
extern "C" FILE *__real_fopen(const char *path, const char *mode);
extern "C" int __real_remove(const char *path);
extern "C" int __real_rename(const char *a, const char *b);

extern "C" FILE *__wrap_fopen(const char *path, const char *mode)
{
    char buf[1024];
    nx_norm(path, buf);
    return __real_fopen(buf, mode);
}

extern "C" int __wrap_remove(const char *path)
{
    char buf[1024];
    nx_norm(path, buf);
    return __real_remove(buf);
}

extern "C" int __wrap_rename(const char *a, const char *b)
{
    char ba[1024], bb[1024];
    nx_norm(a, ba);
    nx_norm(b, bb);
    return __real_rename(ba, bb);
}

extern "C" int nx_rename_compat(const char *a, const char *b)
{
    return __wrap_rename(a, b);
}

// ===========================================================================
// last error
// ===========================================================================
static thread_local DWORD s_lastError = 0;
DWORD GetLastError(void) { return s_lastError; }
void SetLastError(DWORD err) { s_lastError = err; }

// ===========================================================================
// handle plumbing
// ===========================================================================
enum NxHandleType {
    NXH_EVENT = 0x4556u,      // 'EV'
    NXH_THREAD = 0x5448u,     // 'TH'
    NXH_MUTEX = 0x4D58u,      // 'MX'
    NXH_SEMAPHORE = 0x534Du,  // 'SM'
    NXH_FILE = 0x464Cu,       // 'FL'
    NXH_FIND = 0x4644u,       // 'FD'
    NXH_DUMMY = 0x4459u,      // 'DY'
};

struct NxHandleBase {
    uint32_t type;
};

struct NxEvent {
    NxHandleBase h;
    Mutex mutex;
    CondVar cond;
    bool manualReset;
    bool signaled;
};

struct NxThread {
    NxHandleBase h;
    Thread thread;
    LPTHREAD_START_ROUTINE start;
    void *param;
    Mutex mutex;
    CondVar cond;
    bool started;
    bool done;
    bool isMain;
    DWORD exitCode;
    DWORD threadId;
};

struct NxMutexH {
    NxHandleBase h;
    RMutex m;
};

struct NxSemaphore {
    NxHandleBase h;
    Mutex mutex;
    CondVar cond;
    LONG count;
};

struct NxFile {
    NxHandleBase h;
    int fd;
};

struct NxFind {
    NxHandleBase h;
    DIR *dir;
    char dirPath[1024];
    char pattern[256];
};

static NxHandleBase s_dummyHandle = { NXH_DUMMY };

// ===========================================================================
// events
// ===========================================================================
HANDLE CreateEventA(LPSECURITY_ATTRIBUTES, BOOL manualReset, BOOL initialState, LPCSTR)
{
    NxEvent *ev = (NxEvent *)calloc(1, sizeof(NxEvent));
    ev->h.type = NXH_EVENT;
    mutexInit(&ev->mutex);
    condvarInit(&ev->cond);
    ev->manualReset = manualReset != 0;
    ev->signaled = initialState != 0;
    return ev;
}

BOOL SetEvent(HANDLE handle)
{
    NxEvent *ev = (NxEvent *)handle;
    if (!ev || ev->h.type != NXH_EVENT) return FALSE;
    mutexLock(&ev->mutex);
    ev->signaled = true;
    condvarWakeAll(&ev->cond);
    mutexUnlock(&ev->mutex);
    return TRUE;
}

BOOL ResetEvent(HANDLE handle)
{
    NxEvent *ev = (NxEvent *)handle;
    if (!ev || ev->h.type != NXH_EVENT) return FALSE;
    mutexLock(&ev->mutex);
    ev->signaled = false;
    mutexUnlock(&ev->mutex);
    return TRUE;
}

BOOL PulseEvent(HANDLE handle)
{
    NxEvent *ev = (NxEvent *)handle;
    if (!ev || ev->h.type != NXH_EVENT) return FALSE;
    mutexLock(&ev->mutex);
    ev->signaled = false;
    condvarWakeAll(&ev->cond);
    mutexUnlock(&ev->mutex);
    return TRUE;
}

// returns WAIT_OBJECT_0 / WAIT_TIMEOUT
static DWORD nxEventWait(NxEvent *ev, DWORD ms)
{
    DWORD result = WAIT_TIMEOUT;
    mutexLock(&ev->mutex);
    if (ms == INFINITE) {
        while (!ev->signaled)
            condvarWait(&ev->cond, &ev->mutex);
        result = WAIT_OBJECT_0;
    } else if (ms == 0) {
        result = ev->signaled ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
    } else {
        uint64_t deadline = armGetSystemTick() + armNsToTicks((uint64_t)ms * 1000000ull);
        while (!ev->signaled) {
            uint64_t now = armGetSystemTick();
            if (now >= deadline) break;
            uint64_t remainNs = armTicksToNs(deadline - now);
            condvarWaitTimeout(&ev->cond, &ev->mutex, remainNs);
        }
        result = ev->signaled ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
    }
    if (result == WAIT_OBJECT_0 && !ev->manualReset)
        ev->signaled = false;
    mutexUnlock(&ev->mutex);
    return result;
}

// ===========================================================================
// mutex / semaphore handles
// ===========================================================================
HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES, BOOL initialOwner, LPCSTR)
{
    NxMutexH *m = (NxMutexH *)calloc(1, sizeof(NxMutexH));
    m->h.type = NXH_MUTEX;
    rmutexInit(&m->m);
    if (initialOwner)
        rmutexLock(&m->m);
    return m;
}

BOOL ReleaseMutex(HANDLE handle)
{
    NxMutexH *m = (NxMutexH *)handle;
    if (!m || m->h.type != NXH_MUTEX) return FALSE;
    rmutexUnlock(&m->m);
    return TRUE;
}

static DWORD nxMutexWait(NxMutexH *m, DWORD ms)
{
    if (ms == INFINITE) {
        rmutexLock(&m->m);
        return WAIT_OBJECT_0;
    }
    uint64_t deadline = armGetSystemTick() + armNsToTicks((uint64_t)ms * 1000000ull);
    for (;;) {
        if (rmutexTryLock(&m->m))
            return WAIT_OBJECT_0;
        if (armGetSystemTick() >= deadline)
            return WAIT_TIMEOUT;
        svcSleepThread(100000); // 0.1ms
    }
}

HANDLE CreateSemaphoreA(LPSECURITY_ATTRIBUTES, LONG initial, LONG, LPCSTR)
{
    NxSemaphore *s = (NxSemaphore *)calloc(1, sizeof(NxSemaphore));
    s->h.type = NXH_SEMAPHORE;
    mutexInit(&s->mutex);
    condvarInit(&s->cond);
    s->count = initial;
    return s;
}

BOOL ReleaseSemaphore(HANDLE handle, LONG count, LPLONG prev)
{
    NxSemaphore *s = (NxSemaphore *)handle;
    if (!s || s->h.type != NXH_SEMAPHORE) return FALSE;
    mutexLock(&s->mutex);
    if (prev) *prev = s->count;
    s->count += count;
    condvarWakeAll(&s->cond);
    mutexUnlock(&s->mutex);
    return TRUE;
}

static DWORD nxSemWait(NxSemaphore *s, DWORD ms)
{
    DWORD result = WAIT_TIMEOUT;
    mutexLock(&s->mutex);
    if (ms == INFINITE) {
        while (s->count <= 0)
            condvarWait(&s->cond, &s->mutex);
        result = WAIT_OBJECT_0;
    } else {
        uint64_t deadline = armGetSystemTick() + armNsToTicks((uint64_t)ms * 1000000ull);
        while (s->count <= 0) {
            uint64_t now = armGetSystemTick();
            if (now >= deadline) break;
            condvarWaitTimeout(&s->cond, &s->mutex, armTicksToNs(deadline - now));
        }
        result = s->count > 0 ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
    }
    if (result == WAIT_OBJECT_0)
        s->count--;
    mutexUnlock(&s->mutex);
    return result;
}

// ===========================================================================
// threads
// ===========================================================================
static thread_local DWORD s_threadId = 0;
static u32 s_nextThreadId = 1;

DWORD GetCurrentThreadId(void)
{
    if (s_threadId == 0) {
        u64 id = 0;
        svcGetThreadId(&id, CUR_THREAD_HANDLE);
        s_threadId = (DWORD)id;
        if (s_threadId == 0)
            s_threadId = __atomic_add_fetch(&s_nextThreadId, 1, __ATOMIC_SEQ_CST) | 0x40000000u;
    }
    return s_threadId;
}

static NxThread s_mainThread; // filled by nx_wincompat_init_main_thread()

extern "C" void nx_wincompat_init_main_thread(void)
{
    memset(&s_mainThread, 0, sizeof(s_mainThread));
    s_mainThread.h.type = NXH_THREAD;
    s_mainThread.isMain = true;
    s_mainThread.started = true;
    mutexInit(&s_mainThread.mutex);
    condvarInit(&s_mainThread.cond);
    s_mainThread.threadId = GetCurrentThreadId();
}

static void nxThreadEntry(void *arg)
{
    NxThread *t = (NxThread *)arg;
    DWORD code = t->start ? t->start(t->param) : 0;
    mutexLock(&t->mutex);
    t->exitCode = code;
    t->done = true;
    condvarWakeAll(&t->cond);
    mutexUnlock(&t->mutex);
}

HANDLE CreateThread(LPSECURITY_ATTRIBUTES, SIZE_T stackSize, LPTHREAD_START_ROUTINE start,
                    LPVOID param, DWORD flags, LPDWORD threadId)
{
    NxThread *t = (NxThread *)calloc(1, sizeof(NxThread));
    t->h.type = NXH_THREAD;
    t->start = start;
    t->param = param;
    mutexInit(&t->mutex);
    condvarInit(&t->cond);

    size_t stack = stackSize ? (size_t)stackSize : 0x100000; // default 1MB
    if (stack < 0x20000) stack = 0x20000;                    // floor 128KB
    stack = (stack + 0xFFF) & ~(size_t)0xFFF;

    // priority 0x2C == HOS default for app threads; core -2 == same as creator
    Result rc = threadCreate(&t->thread, nxThreadEntry, t, NULL, stack, 0x2C, -2);
    if (R_FAILED(rc)) {
        free(t);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }
    if (threadId) {
        u64 id = 0;
        svcGetThreadId(&id, t->thread.handle);
        t->threadId = (DWORD)id;
        *threadId = t->threadId;
    }
    if (!(flags & CREATE_SUSPENDED)) {
        threadStart(&t->thread);
        t->started = true;
    }
    return t;
}

DWORD ResumeThread(HANDLE handle)
{
    NxThread *t = (NxThread *)handle;
    if (!t || t->h.type != NXH_THREAD) return (DWORD)-1;
    if (!t->started) {
        threadStart(&t->thread);
        t->started = true;
        return 1;
    }
    return 0;
}

DWORD SuspendThread(HANDLE) { return (DWORD)-1; } // fatal-error path only
BOOL TerminateThread(HANDLE, DWORD) { return FALSE; }

void ExitThread(DWORD)
{
    threadExit();
    __builtin_unreachable();
}

HANDLE GetCurrentThread(void) { return (HANDLE)(intptr_t)-2; }
HANDLE GetCurrentProcess(void) { return (HANDLE)(intptr_t)-1; }
DWORD GetCurrentProcessId(void) { return 1; }

BOOL GetExitCodeThread(HANDLE handle, LPDWORD exitCode)
{
    NxThread *t = (NxThread *)handle;
    if (!t || t->h.type != NXH_THREAD) return FALSE;
    if (exitCode) *exitCode = t->done ? t->exitCode : 259 /*STILL_ACTIVE*/;
    return TRUE;
}

static Handle nxThreadKernelHandle(HANDLE handle)
{
    if (handle == (HANDLE)(intptr_t)-2)
        return CUR_THREAD_HANDLE;
    NxThread *t = (NxThread *)handle;
    if (t && t->h.type == NXH_THREAD && !t->isMain)
        return t->thread.handle;
    return CUR_THREAD_HANDLE;
}

BOOL SetThreadPriority(HANDLE handle, int priority)
{
    // Win32 -15..15 (higher = more important) -> HOS 28..59 (lower = more
    // important), around the 0x2C default.
    int prio = 0x2C - priority * 2;
    if (prio < 28) prio = 28;
    if (prio > 59) prio = 59;
    svcSetThreadPriority(nxThreadKernelHandle(handle), (u32)prio);
    return TRUE;
}

int GetThreadPriority(HANDLE) { return THREAD_PRIORITY_NORMAL; }

DWORD_PTR SetThreadAffinityMask(HANDLE handle, DWORD_PTR mask)
{
    u32 m = (u32)(mask & 0x7);
    if (!m) m = 0x7;
    int ideal = __builtin_ctz(m);
    svcSetThreadCoreMask(nxThreadKernelHandle(handle), ideal, m);
    return 0x7;
}

DWORD SetThreadIdealProcessor(HANDLE handle, DWORD processor)
{
    int core = (int)processor;
    if (core > 2) core = 2;
    svcSetThreadCoreMask(nxThreadKernelHandle(handle), core, 0x7);
    return 0;
}

void Sleep(DWORD ms)
{
    svcSleepThread((s64)ms * 1000000ll);
}

BOOL SwitchToThread(void)
{
    svcSleepThread(-1); // yield without core migration
    return TRUE;
}

// Alertable sleep: only used with the ReadFileEx APC emulation below.
static thread_local int s_pendingApcs = 0;

DWORD SleepEx(DWORD ms, BOOL alertable)
{
    if (alertable && s_pendingApcs > 0) {
        s_pendingApcs--;
        return WAIT_IO_COMPLETION;
    }
    if (ms == INFINITE) {
        if (alertable) {
            // The only INFINITE alertable waiter is DB_WaitXFileStage, whose
            // APC is always queued by the same thread before the wait; if we
            // get here the completion already happened synchronously.
            return WAIT_IO_COMPLETION;
        }
        for (;;) svcSleepThread(1000000000ll);
    }
    svcSleepThread((s64)ms * 1000000ll);
    return 0;
}

// ===========================================================================
// TLS (unused by the game; minimal correctness)
// ===========================================================================
static thread_local LPVOID s_tlsSlots[32];
static u32 s_tlsAllocated = 0;

DWORD TlsAlloc(void)
{
    for (int i = 0; i < 32; ++i) {
        u32 bit = 1u << i;
        if (!(__atomic_fetch_or(&s_tlsAllocated, bit, __ATOMIC_SEQ_CST) & bit))
            return (DWORD)i;
    }
    return TLS_OUT_OF_INDEXES;
}
BOOL TlsFree(DWORD index)
{
    if (index >= 32) return FALSE;
    __atomic_fetch_and(&s_tlsAllocated, ~(1u << index), __ATOMIC_SEQ_CST);
    return TRUE;
}
LPVOID TlsGetValue(DWORD index) { return index < 32 ? s_tlsSlots[index] : NULL; }
BOOL TlsSetValue(DWORD index, LPVOID value)
{
    if (index >= 32) return FALSE;
    s_tlsSlots[index] = value;
    return TRUE;
}

// ===========================================================================
// waits / handle close
// ===========================================================================
static DWORD nxThreadWait(NxThread *t, DWORD ms)
{
    DWORD result;
    mutexLock(&t->mutex);
    if (ms == INFINITE) {
        while (!t->done)
            condvarWait(&t->cond, &t->mutex);
        result = WAIT_OBJECT_0;
    } else {
        uint64_t deadline = armGetSystemTick() + armNsToTicks((uint64_t)ms * 1000000ull);
        while (!t->done) {
            uint64_t now = armGetSystemTick();
            if (now >= deadline) break;
            condvarWaitTimeout(&t->cond, &t->mutex, armTicksToNs(deadline - now));
        }
        result = t->done ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
    }
    mutexUnlock(&t->mutex);
    return result;
}

DWORD WaitForSingleObject(HANDLE handle, DWORD ms)
{
    NxHandleBase *h = (NxHandleBase *)handle;
    if (!h) return WAIT_FAILED;
    switch (h->type) {
    case NXH_EVENT:     return nxEventWait((NxEvent *)h, ms);
    case NXH_THREAD:    return nxThreadWait((NxThread *)h, ms);
    case NXH_MUTEX:     return nxMutexWait((NxMutexH *)h, ms);
    case NXH_SEMAPHORE: return nxSemWait((NxSemaphore *)h, ms);
    default:            return WAIT_FAILED;
    }
}

DWORD WaitForMultipleObjects(DWORD count, const HANDLE *handles, BOOL waitAll, DWORD ms)
{
    // No call sites in the game; simple poll fallback for safety.
    (void)waitAll;
    uint64_t deadline = ms == INFINITE ? UINT64_MAX
        : armGetSystemTick() + armNsToTicks((uint64_t)ms * 1000000ull);
    for (;;) {
        for (DWORD i = 0; i < count; ++i)
            if (WaitForSingleObject(handles[i], 0) == WAIT_OBJECT_0)
                return WAIT_OBJECT_0 + i;
        if (armGetSystemTick() >= deadline)
            return WAIT_TIMEOUT;
        svcSleepThread(1000000);
    }
}

BOOL CloseHandle(HANDLE handle)
{
    NxHandleBase *h = (NxHandleBase *)handle;
    if (!h || handle == (HANDLE)(intptr_t)-1 || handle == (HANDLE)(intptr_t)-2 || h == &s_dummyHandle)
        return TRUE;
    switch (h->type) {
    case NXH_EVENT:
    case NXH_MUTEX:
    case NXH_SEMAPHORE:
        free(h);
        return TRUE;
    case NXH_THREAD: {
        NxThread *t = (NxThread *)h;
        if (t->isMain) return TRUE; // never free the main-thread record
        if (t->started)
            threadWaitForExit(&t->thread);
        threadClose(&t->thread);
        free(t);
        return TRUE;
    }
    case NXH_FILE: {
        NxFile *f = (NxFile *)h;
        close(f->fd);
        free(f);
        return TRUE;
    }
    case NXH_FIND:
        return FindClose(handle);
    default:
        return TRUE;
    }
}

BOOL DuplicateHandle(HANDLE, HANDLE src, HANDLE, LPHANDLE dst, DWORD, BOOL, DWORD)
{
    if (!dst) return FALSE;
    if (src == (HANDLE)(intptr_t)-2) {
        // Sys_InitMainThread duplicates the current (main) thread handle.
        *dst = &s_mainThread;
        return TRUE;
    }
    *dst = src;
    return TRUE;
}

// ===========================================================================
// critical sections (recursive, like Win32)
// ===========================================================================
struct NxCritSect {
    uint32_t magic;
    RMutex m;
};
static_assert(sizeof(NxCritSect) <= sizeof(CRITICAL_SECTION), "CRITICAL_SECTION opaque too small");
#define NX_CS_MAGIC 0x43533478u

void InitializeCriticalSection(LPCRITICAL_SECTION cs)
{
    NxCritSect *c = (NxCritSect *)cs;
    rmutexInit(&c->m);
    c->magic = NX_CS_MAGIC;
}

void DeleteCriticalSection(LPCRITICAL_SECTION cs)
{
    ((NxCritSect *)cs)->magic = 0;
}

static inline NxCritSect *nxCs(LPCRITICAL_SECTION cs)
{
    NxCritSect *c = (NxCritSect *)cs;
    if (c->magic != NX_CS_MAGIC)
        InitializeCriticalSection(cs); // lazy init for zero-inited statics
    return c;
}

void EnterCriticalSection(LPCRITICAL_SECTION cs) { rmutexLock(&nxCs(cs)->m); }
void LeaveCriticalSection(LPCRITICAL_SECTION cs) { rmutexUnlock(&nxCs(cs)->m); }
BOOL TryEnterCriticalSection(LPCRITICAL_SECTION cs) { return rmutexTryLock(&nxCs(cs)->m); }

// ===========================================================================
// timing
// ===========================================================================
BOOL QueryPerformanceFrequency(LARGE_INTEGER *freq)
{
    freq->QuadPart = (LONGLONG)armGetSystemTickFreq();
    return TRUE;
}

BOOL QueryPerformanceCounter(LARGE_INTEGER *count)
{
    count->QuadPart = (LONGLONG)armGetSystemTick();
    return TRUE;
}

static uint64_t nxMsNow(void)
{
    return armTicksToNs(armGetSystemTick()) / 1000000ull;
}

static uint64_t s_timeBase; // set on first use so timeGetTime starts small

DWORD timeGetTime(void)
{
    if (s_timeBase == 0)
        s_timeBase = nxMsNow();
    return (DWORD)(nxMsNow() - s_timeBase);
}

DWORD GetTickCount(void) { return timeGetTime(); }
ULONGLONG GetTickCount64(void) { return nxMsNow(); }
UINT timeBeginPeriod(UINT) { return 0; }
UINT timeEndPeriod(UINT) { return 0; }

static void nxFillSystemTime(LPSYSTEMTIME t, bool local)
{
    time_t now = time(NULL);
    struct tm tmv;
    if (local) localtime_r(&now, &tmv);
    else gmtime_r(&now, &tmv);
    t->wYear = (WORD)(tmv.tm_year + 1900);
    t->wMonth = (WORD)(tmv.tm_mon + 1);
    t->wDayOfWeek = (WORD)tmv.tm_wday;
    t->wDay = (WORD)tmv.tm_mday;
    t->wHour = (WORD)tmv.tm_hour;
    t->wMinute = (WORD)tmv.tm_min;
    t->wSecond = (WORD)tmv.tm_sec;
    t->wMilliseconds = 0;
}

void GetSystemTime(LPSYSTEMTIME t) { nxFillSystemTime(t, false); }
void GetLocalTime(LPSYSTEMTIME t)  { nxFillSystemTime(t, true); }

// FILETIME: 100ns units since 1601-01-01; delta to Unix epoch:
#define NX_FILETIME_EPOCH_DELTA 116444736000000000ull

static ULONGLONG nxUnixToFiletime(time_t t)
{
    return (ULONGLONG)t * 10000000ull + NX_FILETIME_EPOCH_DELTA;
}

void GetSystemTimeAsFileTime(LPFILETIME t)
{
    ULONGLONG ft = nxUnixToFiletime(time(NULL));
    t->dwLowDateTime = (DWORD)ft;
    t->dwHighDateTime = (DWORD)(ft >> 32);
}

BOOL SystemTimeToFileTime(const SYSTEMTIME *st, LPFILETIME ft)
{
    struct tm tmv;
    memset(&tmv, 0, sizeof(tmv));
    tmv.tm_year = st->wYear - 1900;
    tmv.tm_mon = st->wMonth - 1;
    tmv.tm_mday = st->wDay;
    tmv.tm_hour = st->wHour;
    tmv.tm_min = st->wMinute;
    tmv.tm_sec = st->wSecond;
    ULONGLONG v = nxUnixToFiletime(mktime(&tmv));
    ft->dwLowDateTime = (DWORD)v;
    ft->dwHighDateTime = (DWORD)(v >> 32);
    return TRUE;
}

BOOL FileTimeToSystemTime(const FILETIME *ft, LPSYSTEMTIME st)
{
    ULONGLONG v = ((ULONGLONG)ft->dwHighDateTime << 32) | ft->dwLowDateTime;
    time_t t = (time_t)((v - NX_FILETIME_EPOCH_DELTA) / 10000000ull);
    struct tm tmv;
    gmtime_r(&t, &tmv);
    st->wYear = (WORD)(tmv.tm_year + 1900);
    st->wMonth = (WORD)(tmv.tm_mon + 1);
    st->wDayOfWeek = (WORD)tmv.tm_wday;
    st->wDay = (WORD)tmv.tm_mday;
    st->wHour = (WORD)tmv.tm_hour;
    st->wMinute = (WORD)tmv.tm_min;
    st->wSecond = (WORD)tmv.tm_sec;
    st->wMilliseconds = 0;
    return TRUE;
}

LONG CompareFileTime(const FILETIME *a, const FILETIME *b)
{
    ULONGLONG va = ((ULONGLONG)a->dwHighDateTime << 32) | a->dwLowDateTime;
    ULONGLONG vb = ((ULONGLONG)b->dwHighDateTime << 32) | b->dwLowDateTime;
    if (va < vb) return -1;
    if (va > vb) return 1;
    return 0;
}

// ===========================================================================
// file I/O
// ===========================================================================
HANDLE CreateFileA(LPCSTR name, DWORD access, DWORD, LPSECURITY_ATTRIBUTES,
                   DWORD disposition, DWORD, HANDLE)
{
    char path[1024];
    nx_norm(name, path);

    int flags = 0;
    bool rd = (access & GENERIC_READ) != 0;
    bool wr = (access & GENERIC_WRITE) != 0;
    flags = rd && wr ? O_RDWR : wr ? O_WRONLY : O_RDONLY;

    switch (disposition) {
    case CREATE_NEW:        flags |= O_CREAT | O_EXCL; break;
    case CREATE_ALWAYS:     flags |= O_CREAT | O_TRUNC; break;
    case OPEN_ALWAYS:       flags |= O_CREAT; break;
    case TRUNCATE_EXISTING: flags |= O_TRUNC; break;
    case OPEN_EXISTING:     break;
    }

    int fd = open(path, flags, 0666);
    if (fd < 0) {
        SetLastError(errno == ENOENT ? ERROR_FILE_NOT_FOUND : ERROR_ACCESS_DENIED);
        return INVALID_HANDLE_VALUE;
    }
    NxFile *f = (NxFile *)calloc(1, sizeof(NxFile));
    f->h.type = NXH_FILE;
    f->fd = fd;
    return f;
}

static NxFile *nxFile(HANDLE h)
{
    NxFile *f = (NxFile *)h;
    return (f && h != INVALID_HANDLE_VALUE && f->h.type == NXH_FILE) ? f : NULL;
}

static ssize_t nxReadAt(NxFile *f, void *buf, DWORD toRead, off_t offset, bool *atEof);

BOOL ReadFile(HANDLE handle, LPVOID buf, DWORD toRead, LPDWORD readOut, LPOVERLAPPED ov)
{
    NxFile *f = nxFile(handle);
    if (!f) { SetLastError(ERROR_INVALID_HANDLE); return FALSE; }
    ssize_t n;
    if (ov) {
        off_t offset = (off_t)(((uint64_t)ov->OffsetHigh << 32) | ov->Offset);
        bool atEof = false;
        // Unlike ReadFileEx, plain ReadFile reports EOF as success with zero
        // bytes transferred.
        n = nxReadAt(f, buf, toRead, offset, &atEof);
        if (n >= 0) {
            ov->Internal = 0;
            ov->InternalHigh = (ULONG_PTR)n;
            if (ov->hEvent) SetEvent(ov->hEvent);
        }
    } else {
        n = read(f->fd, buf, toRead);
    }
    if (n < 0) { SetLastError(ERROR_ACCESS_DENIED); return FALSE; }
    if (readOut) *readOut = (DWORD)n;
    return TRUE;
}

// Positional read with Windows end-of-file semantics.
//
// Horizon's fs errors out when asked to read past the end of a file, whereas
// Win32 simply reports EOF. Clamp the request to what is actually there, and
// tell the caller when the read started at/after EOF.
static ssize_t nxReadAt(NxFile *f, void *buf, DWORD toRead, off_t offset, bool *atEof)
{
    struct stat st;
    *atEof = false;
    if (fstat(f->fd, &st) != 0)
        return -1;
    if (offset >= st.st_size) {
        *atEof = true;
        return 0;
    }
    off_t remain = st.st_size - offset;
    if ((off_t)toRead > remain)
        toRead = (DWORD)remain;
    return pread(f->fd, buf, toRead, offset);
}

BOOL ReadFileEx(HANDLE handle, LPVOID buf, DWORD toRead, LPOVERLAPPED ov,
                LPOVERLAPPED_COMPLETION_ROUTINE completion)
{
    NxFile *f = nxFile(handle);
    if (!f || !ov) { SetLastError(ERROR_INVALID_HANDLE); return FALSE; }
    off_t offset = (off_t)(((uint64_t)ov->OffsetHigh << 32) | ov->Offset);

    bool atEof = false;
    ssize_t n = nxReadAt(f, buf, toRead, offset, &atEof);
    if (n < 0) { SetLastError(ERROR_ACCESS_DENIED); return FALSE; }
    // A read beginning at or past EOF fails with ERROR_HANDLE_EOF on Windows.
    // The fastfile loader depends on precisely this: db_file_load.cpp only
    // tolerates a failed read when GetLastError() == ERROR_HANDLE_EOF (38),
    // and it always queues one read beyond the end of the file.
    if (atEof || n == 0) { SetLastError(ERROR_HANDLE_EOF); return FALSE; }

    ov->Internal = 0;
    ov->InternalHigh = (ULONG_PTR)n;
    // Emulate the APC: deliver the completion now, credit one alertable wait.
    if (completion) completion(0, (DWORD)n, ov);
    s_pendingApcs++;
    return TRUE;
}

BOOL WriteFile(HANDLE handle, LPCVOID buf, DWORD toWrite, LPDWORD written, LPOVERLAPPED ov)
{
    NxFile *f = nxFile(handle);
    if (!f) { SetLastError(ERROR_INVALID_HANDLE); return FALSE; }
    ssize_t n;
    if (ov) {
        off_t offset = (off_t)(((uint64_t)ov->OffsetHigh << 32) | ov->Offset);
        n = pwrite(f->fd, buf, toWrite, offset);
        if (n >= 0) {
            ov->InternalHigh = (ULONG_PTR)n;
            if (ov->hEvent) SetEvent(ov->hEvent);
        }
    } else {
        n = write(f->fd, buf, toWrite);
    }
    if (n < 0) { SetLastError(ERROR_ACCESS_DENIED); return FALSE; }
    if (written) *written = (DWORD)n;
    return TRUE;
}

BOOL GetOverlappedResult(HANDLE, LPOVERLAPPED ov, LPDWORD transferred, BOOL)
{
    if (!ov) return FALSE;
    if (transferred) *transferred = (DWORD)ov->InternalHigh;
    return TRUE;
}

DWORD SetFilePointer(HANDLE handle, LONG dist, PLONG distHigh, DWORD method)
{
    NxFile *f = nxFile(handle);
    if (!f) return INVALID_SET_FILE_POINTER;
    int whence = method == FILE_BEGIN ? SEEK_SET : method == FILE_CURRENT ? SEEK_CUR : SEEK_END;
    int64_t offset = distHigh ? (int64_t)(((uint64_t)(uint32_t)*distHigh << 32) | (uint32_t)dist)
                              : (int64_t)dist;
    off_t r = lseek(f->fd, (off_t)offset, whence);
    if (r < 0) return INVALID_SET_FILE_POINTER;
    if (distHigh) *distHigh = (LONG)((uint64_t)r >> 32);
    return (DWORD)r;
}

BOOL SetFilePointerEx(HANDLE handle, LARGE_INTEGER dist, PLARGE_INTEGER newPos, DWORD method)
{
    NxFile *f = nxFile(handle);
    if (!f) return FALSE;
    int whence = method == FILE_BEGIN ? SEEK_SET : method == FILE_CURRENT ? SEEK_CUR : SEEK_END;
    off_t r = lseek(f->fd, (off_t)dist.QuadPart, whence);
    if (r < 0) return FALSE;
    if (newPos) newPos->QuadPart = (LONGLONG)r;
    return TRUE;
}

BOOL SetEndOfFile(HANDLE handle)
{
    NxFile *f = nxFile(handle);
    if (!f) return FALSE;
    off_t pos = lseek(f->fd, 0, SEEK_CUR);
    return ftruncate(f->fd, pos) == 0;
}

DWORD GetFileSize(HANDLE handle, LPDWORD sizeHigh)
{
    NxFile *f = nxFile(handle);
    if (!f) return INVALID_FILE_SIZE;
    struct stat st;
    if (fstat(f->fd, &st) != 0) return INVALID_FILE_SIZE;
    if (sizeHigh) *sizeHigh = (DWORD)((uint64_t)st.st_size >> 32);
    return (DWORD)st.st_size;
}

BOOL GetFileSizeEx(HANDLE handle, PLARGE_INTEGER size)
{
    NxFile *f = nxFile(handle);
    if (!f) return FALSE;
    struct stat st;
    if (fstat(f->fd, &st) != 0) return FALSE;
    size->QuadPart = (LONGLONG)st.st_size;
    return TRUE;
}

BOOL FlushFileBuffers(HANDLE handle)
{
    NxFile *f = nxFile(handle);
    if (!f) return FALSE;
    fsync(f->fd);
    return TRUE;
}

BOOL DeleteFileA(LPCSTR name)
{
    char path[1024];
    nx_norm(name, path);
    return remove(path) == 0;
}

BOOL MoveFileA(LPCSTR from, LPCSTR to)
{
    return nx_rename_compat(from, to) == 0;
}

BOOL CopyFileA(LPCSTR from, LPCSTR to, BOOL failIfExists)
{
    char src[1024], dst[1024];
    nx_norm(from, src);
    nx_norm(to, dst);
    if (failIfExists) {
        struct stat st;
        if (stat(dst, &st) == 0) return FALSE;
    }
    FILE *in = fopen(src, "rb");
    if (!in) return FALSE;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return FALSE; }
    char buf[65536];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
        fwrite(buf, 1, n, out);
    fclose(in);
    fclose(out);
    return TRUE;
}

BOOL CreateDirectoryA(LPCSTR name, LPSECURITY_ATTRIBUTES)
{
    char path[1024];
    nx_norm(name, path);
    if (mkdir(path, 0777) == 0) return TRUE;
    SetLastError(errno == EEXIST ? ERROR_ALREADY_EXISTS : ERROR_PATH_NOT_FOUND);
    return FALSE;
}

BOOL RemoveDirectoryA(LPCSTR name)
{
    char path[1024];
    nx_norm(name, path);
    return rmdir(path) == 0;
}

DWORD GetFileAttributesA(LPCSTR name)
{
    char path[1024];
    nx_norm(name, path);
    struct stat st;
    if (stat(path, &st) != 0) {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return INVALID_FILE_ATTRIBUTES;
    }
    return S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
}

BOOL SetFileAttributesA(LPCSTR, DWORD) { return TRUE; }

DWORD GetCurrentDirectoryA(DWORD len, LPSTR buf)
{
    if (!getcwd(buf, len)) return 0;
    return (DWORD)strlen(buf);
}

BOOL SetCurrentDirectoryA(LPCSTR path)
{
    char buf[1024];
    nx_norm(path, buf);
    return chdir(buf) == 0;
}

DWORD GetFullPathNameA(LPCSTR name, DWORD len, LPSTR buf, LPSTR *filePart)
{
    char norm[1024];
    nx_norm(name, norm);
    strncpy(buf, norm, len);
    buf[len - 1] = 0;
    if (filePart) {
        char *slash = strrchr(buf, '/');
        *filePart = slash ? slash + 1 : buf;
    }
    return (DWORD)strlen(buf);
}

BOOL GetDiskFreeSpaceA(LPCSTR, LPDWORD sectorsPerCluster, LPDWORD bytesPerSector,
                       LPDWORD freeClusters, LPDWORD totalClusters)
{
    // report 4GB free / 8GB total, in 32KB clusters
    if (sectorsPerCluster) *sectorsPerCluster = 64;
    if (bytesPerSector) *bytesPerSector = 512;
    if (freeClusters) *freeClusters = 131072;
    if (totalClusters) *totalClusters = 262144;
    return TRUE;
}

// ----- FindFirstFile ---------------------------------------------------------
extern "C" int nx_wildcard_match(const char *pattern, const char *str)
{
    // case-insensitive * and ? matching
    while (*pattern) {
        if (*pattern == '*') {
            pattern++;
            if (!*pattern) return 1;
            for (const char *s = str; ; ++s) {
                if (nx_wildcard_match(pattern, s)) return 1;
                if (!*s) return 0;
            }
        }
        if (!*str) return 0;
        if (*pattern != '?' && tolower((unsigned char)*pattern) != tolower((unsigned char)*str))
            return 0;
        pattern++;
        str++;
    }
    return *str == 0;
}

static void nxFillFindData(const char *dirPath, const char *name, LPWIN32_FIND_DATAA data)
{
    memset(data, 0, sizeof(*data));
    strncpy(data->cFileName, name, sizeof(data->cFileName) - 1);
    char full[1200];
    snprintf(full, sizeof(full), "%s/%s", dirPath, name);
    struct stat st;
    if (stat(full, &st) == 0) {
        data->dwFileAttributes = S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
        data->nFileSizeLow = (DWORD)st.st_size;
        data->nFileSizeHigh = (DWORD)((uint64_t)st.st_size >> 32);
        ULONGLONG wt = nxUnixToFiletime(st.st_mtime);
        data->ftLastWriteTime.dwLowDateTime = (DWORD)wt;
        data->ftLastWriteTime.dwHighDateTime = (DWORD)(wt >> 32);
        data->ftLastAccessTime = data->ftLastWriteTime;
        data->ftCreationTime = data->ftLastWriteTime;
    } else {
        data->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    }
}

HANDLE FindFirstFileA(LPCSTR pattern, LPWIN32_FIND_DATAA data)
{
    char norm[1024];
    nx_norm(pattern, norm);

    char dirPath[1024];
    const char *filePat;
    char *slash = strrchr(norm, '/');
    if (slash) {
        size_t dirLen = (size_t)(slash - norm);
        memcpy(dirPath, norm, dirLen);
        dirPath[dirLen] = 0;
        filePat = slash + 1;
    } else {
        strcpy(dirPath, ".");
        filePat = norm;
    }

    DIR *dir = opendir(dirPath);
    if (!dir) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    NxFind *find = (NxFind *)calloc(1, sizeof(NxFind));
    find->h.type = NXH_FIND;
    find->dir = dir;
    strncpy(find->dirPath, dirPath, sizeof(find->dirPath) - 1);
    strncpy(find->pattern, filePat, sizeof(find->pattern) - 1);

    if (FindNextFileA(find, data))
        return find;

    closedir(dir);
    free(find);
    SetLastError(ERROR_FILE_NOT_FOUND);
    return INVALID_HANDLE_VALUE;
}

BOOL FindNextFileA(HANDLE handle, LPWIN32_FIND_DATAA data)
{
    NxFind *find = (NxFind *)handle;
    if (!find || handle == INVALID_HANDLE_VALUE || find->h.type != NXH_FIND) return FALSE;
    struct dirent *ent;
    while ((ent = readdir(find->dir)) != NULL) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
            continue;
        if (nx_wildcard_match(find->pattern, ent->d_name)) {
            nxFillFindData(find->dirPath, ent->d_name, data);
            return TRUE;
        }
    }
    SetLastError(ERROR_NO_MORE_FILES);
    return FALSE;
}

BOOL FindClose(HANDLE handle)
{
    NxFind *find = (NxFind *)handle;
    if (!find || handle == INVALID_HANDLE_VALUE || find->h.type != NXH_FIND) return FALSE;
    closedir(find->dir);
    free(find);
    return TRUE;
}

// ----- MSVC _find*/direct.h/io.h family --------------------------------------
//
// The engine stores these handles in plain `int` variables (win_common.cpp:
// `int hFile = _findfirst64i32(...)`), which is fine when intptr_t is 32 bits
// but truncates a real pointer on LP64. Hand out small table indices instead
// so the handle always survives a round trip through an int.
#define NX_MAX_FINDS 32
static NxFind *s_findSlots[NX_MAX_FINDS];
static Mutex s_findLock;
static bool s_findLockInit;

static void nxFindLock(void)
{
    if (!s_findLockInit) {
        mutexInit(&s_findLock);
        s_findLockInit = true;
    }
    mutexLock(&s_findLock);
}

static NxFind *nxFindFromSlot(intptr_t handle)
{
    if (handle < 0 || handle >= NX_MAX_FINDS)
        return NULL;
    return s_findSlots[handle];
}

static void nxFillMsvcFindData(const WIN32_FIND_DATAA *wd, struct _finddata64i32_t *data)
{
    data->attrib = (wd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? _A_SUBDIR : _A_NORMAL;
    data->size = wd->nFileSizeLow;
    data->time_write = (long long)((((ULONGLONG)wd->ftLastWriteTime.dwHighDateTime << 32
        | wd->ftLastWriteTime.dwLowDateTime) - NX_FILETIME_EPOCH_DELTA) / 10000000ull);
    data->time_create = data->time_access = data->time_write;
    strncpy(data->name, wd->cFileName, sizeof(data->name) - 1);
    data->name[sizeof(data->name) - 1] = 0;
}

extern "C" intptr_t nx_findfirst64i32(const char *pattern, struct _finddata64i32_t *data)
{
    WIN32_FIND_DATAA wd;
    HANDLE h = FindFirstFileA(pattern, &wd);
    if (h == INVALID_HANDLE_VALUE)
        return -1;

    nxFindLock();
    int slot = -1;
    for (int i = 0; i < NX_MAX_FINDS; ++i) {
        if (!s_findSlots[i]) {
            s_findSlots[i] = (NxFind *)h;
            slot = i;
            break;
        }
    }
    mutexUnlock(&s_findLock);

    if (slot < 0) {
        FindClose(h);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return -1;
    }
    nxFillMsvcFindData(&wd, data);
    return (intptr_t)slot;
}

extern "C" int nx_findnext64i32(intptr_t handle, struct _finddata64i32_t *data)
{
    NxFind *find = nxFindFromSlot(handle);
    if (!find)
        return -1;
    WIN32_FIND_DATAA wd;
    if (!FindNextFileA(find, &wd))
        return -1;
    nxFillMsvcFindData(&wd, data);
    return 0;
}

extern "C" int nx_findclose(intptr_t handle)
{
    NxFind *find = nxFindFromSlot(handle);
    if (!find)
        return -1;
    nxFindLock();
    s_findSlots[handle] = NULL;
    mutexUnlock(&s_findLock);
    return FindClose(find) ? 0 : -1;
}

extern "C" int nx_mkdir_compat(const char *path)
{
    char buf[1024];
    nx_norm(path, buf);
    return mkdir(buf, 0777);
}

extern "C" char *nx_getcwd_compat(char *buf, int size)
{
    return getcwd(buf, (size_t)size);
}

extern "C" int nx_chdir_compat(const char *path)
{
    char buf[1024];
    nx_norm(path, buf);
    return chdir(buf);
}

extern "C" int nx_rmdir_compat(const char *path)
{
    char buf[1024];
    nx_norm(path, buf);
    return rmdir(buf);
}

extern "C" int nx_access_compat(const char *path, int mode)
{
    char buf[1024];
    nx_norm(path, buf);
    return access(buf, mode == 0 ? F_OK : mode);
}

extern "C" int nx_unlink_compat(const char *path)
{
    char buf[1024];
    nx_norm(path, buf);
    return unlink(buf);
}

// ===========================================================================
// modules
// ===========================================================================
extern "C" const char *nx_get_install_dir(void); // nx_main.cpp

DWORD GetModuleFileNameA(HMODULE, LPSTR buf, DWORD len)
{
    snprintf(buf, len, "%s/KisakBlack.nro", nx_get_install_dir());
    return (DWORD)strlen(buf);
}

HMODULE GetModuleHandleA(LPCSTR) { return (HMODULE)&s_dummyHandle; }
HMODULE LoadLibraryA(LPCSTR)
{
    SetLastError(ERROR_FILE_NOT_FOUND);
    return NULL;
}
BOOL FreeLibrary(HMODULE) { return TRUE; }
void *GetProcAddress(HMODULE, LPCSTR) { return NULL; }

// ===========================================================================
// memory
// ===========================================================================
// The engine's Z_Virtual* allocator leans on real VirtualQuery semantics:
// VirtualQuery(base) must report the whole reserved region as one committed
// block with AllocationBase == BaseAddress == base (see Z_VirtualFree /
// Z_VirtualCommit in com_memory.cpp). Track each VirtualAlloc region and
// hand out page-aligned memory so those invariants hold.
#define NX_PAGE 0x1000u
#define NX_MAX_VMEM 512
struct NxVMemRegion { uintptr_t base; size_t size; };
static NxVMemRegion s_vmem[NX_MAX_VMEM];
static Mutex s_vmemLock;
static bool s_vmemLockInit;

static void nxVmemLock(void)
{
    if (!s_vmemLockInit) { mutexInit(&s_vmemLock); s_vmemLockInit = true; }
    mutexLock(&s_vmemLock);
}

LPVOID VirtualAlloc(LPVOID address, SIZE_T size, DWORD type, DWORD)
{
    // Committing inside a prior reservation: memory is already backed.
    if (address && (type & MEM_COMMIT) && !(type & MEM_RESERVE))
        return address;
    if (type & (MEM_RESERVE | MEM_COMMIT)) {
        size_t rounded = (size + (NX_PAGE - 1)) & ~(size_t)(NX_PAGE - 1);
        if (rounded == 0) rounded = NX_PAGE;
        void *p = memalign(NX_PAGE, rounded);
        if (!p) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return NULL; }
        memset(p, 0, rounded);
        nxVmemLock();
        for (int i = 0; i < NX_MAX_VMEM; ++i) {
            if (!s_vmem[i].base) { s_vmem[i].base = (uintptr_t)p; s_vmem[i].size = rounded; break; }
        }
        mutexUnlock(&s_vmemLock);
        return p;
    }
    return NULL;
}

BOOL VirtualFree(LPVOID address, SIZE_T, DWORD type)
{
    if (type & MEM_RELEASE) {
        nxVmemLock();
        for (int i = 0; i < NX_MAX_VMEM; ++i) {
            if (s_vmem[i].base == (uintptr_t)address) { s_vmem[i].base = 0; s_vmem[i].size = 0; break; }
        }
        mutexUnlock(&s_vmemLock);
        free(address);
    }
    // MEM_DECOMMIT keeps the mapping (our reserve already committed real
    // memory), so it is a no-op.
    return TRUE;
}

BOOL VirtualProtect(LPVOID, SIZE_T, DWORD, PDWORD oldProtect)
{
    if (oldProtect) *oldProtect = PAGE_READWRITE;
    return TRUE;
}

SIZE_T VirtualQuery(LPCVOID address, PMEMORY_BASIC_INFORMATION buf, SIZE_T)
{
    uintptr_t a = (uintptr_t)address;
    uintptr_t rbase = 0;
    size_t rsize = 0;
    nxVmemLock();
    for (int i = 0; i < NX_MAX_VMEM; ++i) {
        if (s_vmem[i].base && a >= s_vmem[i].base && a < s_vmem[i].base + s_vmem[i].size) {
            rbase = s_vmem[i].base;
            rsize = s_vmem[i].size;
            break;
        }
    }
    mutexUnlock(&s_vmemLock);

    uintptr_t pageAddr = a & ~(uintptr_t)(NX_PAGE - 1);
    buf->AllocationProtect = PAGE_READWRITE;
    buf->State = MEM_COMMIT; // 0x1000
    buf->Protect = PAGE_READWRITE;
    buf->Type = 0x20000; // MEM_PRIVATE
    if (rbase) {
        // Report the rest of this region as one committed block, matching
        // Windows' coalescing of same-state pages.
        buf->BaseAddress = (PVOID)pageAddr;
        buf->AllocationBase = (PVOID)rbase;
        buf->RegionSize = (rbase + rsize) - pageAddr;
    } else {
        // Untracked memory (regular heap): report a single committed page so
        // unrelated callers keep working; the Z_Virtual* asserts only ever
        // query VirtualAlloc'd regions.
        buf->BaseAddress = (PVOID)pageAddr;
        buf->AllocationBase = (PVOID)pageAddr;
        buf->RegionSize = NX_PAGE;
    }
    return sizeof(*buf);
}

HANDLE GetProcessHeap(void) { return (HANDLE)&s_dummyHandle; }
LPVOID HeapAlloc(HANDLE, DWORD flags, SIZE_T size)
{
    return (flags & HEAP_ZERO_MEMORY) ? calloc(1, size) : malloc(size);
}
LPVOID HeapReAlloc(HANDLE, DWORD, LPVOID mem, SIZE_T size) { return realloc(mem, size); }
BOOL HeapFree(HANDLE, DWORD, LPVOID mem) { free(mem); return TRUE; }

HGLOBAL GlobalAlloc(UINT flags, SIZE_T size)
{
    return (HGLOBAL)((flags & GMEM_ZEROINIT) ? calloc(1, size) : malloc(size));
}
HGLOBAL GlobalFree(HGLOBAL mem) { free(mem); return NULL; }
LPVOID GlobalLock(HGLOBAL mem) { return mem; }
BOOL GlobalUnlock(HGLOBAL) { return TRUE; }

// libnx reserves the whole available region as the newlib heap at startup, so
// the kernel's "used memory" is ~100% from the moment we run. Reporting
// total-minus-used as free would say ~0 MB and trip the engine's low-memory
// bail-out; what the game actually cares about is how much it can still
// allocate, i.e. the unused portion of the heap.
extern "C" char *fake_heap_start;
extern "C" char *fake_heap_end;

extern "C" void nx_mem_status(uint64_t *totalOut, uint64_t *availOut)
{
    u64 totalMem = 0;
    svcGetInfo(&totalMem, InfoType_TotalMemorySize, CUR_PROCESS_HANDLE, 0);

    size_t heapSize = (size_t)(fake_heap_end - fake_heap_start);
    struct mallinfo mi = mallinfo();
    size_t usedHeap = (size_t)mi.uordblks;
    size_t freeHeap = heapSize > usedHeap ? heapSize - usedHeap : 0;

    if (totalOut) *totalOut = totalMem ? totalMem : (u64)heapSize;
    if (availOut) *availOut = freeHeap;
}

static void nxMemStatus(SIZE_T *total, SIZE_T *avail)
{
    uint64_t t = 0, a = 0;
    nx_mem_status(&t, &a);
    *total = (SIZE_T)t;
    *avail = (SIZE_T)a;
}

void GlobalMemoryStatus(LPMEMORYSTATUS status)
{
    SIZE_T total, avail;
    nxMemStatus(&total, &avail);
    status->dwLength = sizeof(*status);
    status->dwMemoryLoad = total ? (DWORD)(100 - avail * 100 / total) : 0;
    status->dwTotalPhys = total;
    status->dwAvailPhys = avail;
    status->dwTotalPageFile = total;
    status->dwAvailPageFile = avail;
    status->dwTotalVirtual = total;
    status->dwAvailVirtual = avail;
}

BOOL GlobalMemoryStatusEx(LPMEMORYSTATUSEX status)
{
    SIZE_T total, avail;
    nxMemStatus(&total, &avail);
    status->dwMemoryLoad = total ? (DWORD)(100 - avail * 100 / total) : 0;
    status->ullTotalPhys = total;
    status->ullAvailPhys = avail;
    status->ullTotalPageFile = total;
    status->ullAvailPageFile = avail;
    status->ullTotalVirtual = total;
    status->ullAvailVirtual = avail;
    status->ullAvailExtendedVirtual = 0;
    return TRUE;
}

BOOL IsBadReadPtr(const void *p, UINT_PTR) { return p == NULL; }
BOOL IsBadWritePtr(void *p, UINT_PTR) { return p == NULL; }

// ===========================================================================
// system info / process
// ===========================================================================
void GetSystemInfo(LPSYSTEM_INFO info)
{
    memset(info, 0, sizeof(*info));
    info->dwPageSize = 0x1000;
    info->dwNumberOfProcessors = 3; // cores available to applications
    info->dwActiveProcessorMask = 0x7;
    info->dwAllocationGranularity = 0x10000;
    info->wProcessorLevel = 8; // ARMv8
}

BOOL GetVersionExA(LPOSVERSIONINFOA info)
{
    info->dwMajorVersion = 6;
    info->dwMinorVersion = 1;
    info->dwBuildNumber = 7601;
    info->dwPlatformId = VER_PLATFORM_WIN32_NT;
    strcpy(info->szCSDVersion, "Service Pack 1");
    return TRUE;
}

DWORD GetEnvironmentVariableA(LPCSTR name, LPSTR buf, DWORD len)
{
    const char *v = getenv(name);
    if (!v) return 0;
    strncpy(buf, v, len);
    if (len) buf[len - 1] = 0;
    return (DWORD)strlen(v);
}

BOOL SetEnvironmentVariableA(LPCSTR name, LPCSTR value)
{
    return setenv(name, value ? value : "", 1) == 0;
}

static char s_cmdline[1024] = "";
extern "C" void nx_set_command_line(const char *cmdline)
{
    strncpy(s_cmdline, cmdline, sizeof(s_cmdline) - 1);
}
LPSTR GetCommandLineA(void) { return s_cmdline; }

DWORD ExpandEnvironmentStringsA(LPCSTR src, LPSTR dst, DWORD len)
{
    strncpy(dst, src, len);
    if (len) dst[len - 1] = 0;
    return (DWORD)strlen(dst);
}

void ExitProcess(UINT code)
{
    exit((int)code);
}

BOOL TerminateProcess(HANDLE, UINT code) { exit((int)code); }
UINT SetErrorMode(UINT) { return 0; }
BOOL IsDebuggerPresent(void) { return FALSE; }

void OutputDebugStringA(LPCSTR str)
{
    if (str) {
        fputs(str, stderr);
        svcOutputDebugString(str, strlen(str));
    }
}

void DebugBreak(void) { __builtin_trap(); }

BOOL CreateProcessA(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD,
                    LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION)
{
    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
}

void GetStartupInfoA(LPSTARTUPINFOA si) { memset(si, 0, sizeof(*si)); si->cb = sizeof(*si); }

HANDLE OpenProcess(DWORD, BOOL, DWORD) { return &s_dummyHandle; }
BOOL OpenProcessToken(HANDLE, DWORD, PHANDLE) { return FALSE; }
BOOL OpenThreadToken(HANDLE, DWORD, BOOL, PHANDLE) { return FALSE; }
BOOL GetProcessAffinityMask(HANDLE, DWORD_PTR *processMask, DWORD_PTR *systemMask)
{
    if (processMask) *processMask = 0x7;
    if (systemMask) *systemMask = 0x7;
    return TRUE;
}
BOOL SetProcessAffinityMask(HANDLE, DWORD_PTR) { return TRUE; }

LPTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER) { return NULL; }

void RaiseException(DWORD code, DWORD, DWORD, const ULONG_PTR *)
{
    fprintf(stderr, "RaiseException(0x%x) -> abort\n", (unsigned)code);
    abort();
}

// wincrypt subset
BOOL CryptAcquireContextA(HCRYPTPROV *prov, LPCSTR, LPCSTR, DWORD, DWORD)
{
    if (prov) *prov = 1;
    return TRUE;
}
BOOL CryptGenRandom(HCRYPTPROV, DWORD len, BYTE *buffer)
{
    randomGet(buffer, len);
    return TRUE;
}
BOOL CryptReleaseContext(HCRYPTPROV, DWORD) { return TRUE; }

// Psapi
BOOL GetProcessMemoryInfo(HANDLE, PPROCESS_MEMORY_COUNTERS counters, DWORD cb)
{
    SIZE_T total, avail;
    nxMemStatus(&total, &avail);
    memset(counters, 0, cb);
    counters->cb = cb;
    counters->WorkingSetSize = total - avail;
    counters->PeakWorkingSetSize = total - avail;
    counters->PagefileUsage = total - avail;
    return TRUE;
}

// TlHelp32
HANDLE CreateToolhelp32Snapshot(DWORD, DWORD) { return INVALID_HANDLE_VALUE; }
BOOL Process32First(HANDLE, LPPROCESSENTRY32) { return FALSE; }
BOOL Process32Next(HANDLE, LPPROCESSENTRY32) { return FALSE; }
BOOL Module32First(HANDLE, LPMODULEENTRY32) { return FALSE; }
BOOL Module32Next(HANDLE, LPMODULEENTRY32) { return FALSE; }

// ShlObj
HRESULT SHGetFolderPathA(HWND, int csidl, HANDLE, DWORD, LPSTR path)
{
    (void)csidl;
    snprintf(path, MAX_PATH, "%s/docs", nx_get_install_dir());
    mkdir(path, 0777);
    return S_OK;
}

// ===========================================================================
// strings
// ===========================================================================
int MultiByteToWideChar(UINT, DWORD, LPCSTR src, int srcLen, LPWSTR dst, int dstLen)
{
    int n = srcLen < 0 ? (int)strlen(src) + 1 : srcLen;
    if (dstLen == 0) return n;
    int count = n < dstLen ? n : dstLen;
    for (int i = 0; i < count; ++i)
        dst[i] = (WCHAR)(unsigned char)src[i];
    return count;
}

int WideCharToMultiByte(UINT, DWORD, LPCWSTR src, int srcLen, LPSTR dst, int dstLen, LPCSTR, LPBOOL)
{
    int n;
    if (srcLen < 0) {
        n = 0;
        while (src[n]) n++;
        n++;
    } else {
        n = srcLen;
    }
    if (dstLen == 0) return n;
    int count = n < dstLen ? n : dstLen;
    for (int i = 0; i < count; ++i)
        dst[i] = (char)(src[i] < 256 ? src[i] : '?');
    return count;
}

int lstrlenA(LPCSTR s) { return (int)strlen(s); }
LPSTR lstrcpyA(LPSTR dst, LPCSTR src) { return strcpy(dst, src); }
int lstrcmpA(LPCSTR a, LPCSTR b) { return strcmp(a, b); }
int lstrcmpiA(LPCSTR a, LPCSTR b) { return strcasecmp(a, b); }
LPSTR CharLowerA(LPSTR s) { for (char *p = s; *p; ++p) *p = (char)tolower((unsigned char)*p); return s; }
LPSTR CharUpperA(LPSTR s) { for (char *p = s; *p; ++p) *p = (char)toupper((unsigned char)*p); return s; }
DWORD GetUserNameA_shim(LPSTR buf, LPDWORD len)
{
    strncpy(buf, "Player", *len);
    *len = 7;
    return TRUE;
}

DWORD FormatMessageA(DWORD, LPCVOID, DWORD messageId, DWORD, LPSTR buffer, DWORD size, va_list *)
{
    snprintf(buffer, size, "error %u", (unsigned)messageId);
    return (DWORD)strlen(buffer);
}

// ===========================================================================
// COM leftovers
// ===========================================================================
HRESULT CoInitialize(LPVOID) { return S_OK; }
HRESULT CoInitializeEx(LPVOID, DWORD) { return S_OK; }
void CoUninitialize(void) {}
HRESULT CoCreateGuid(GUID *guid)
{
    randomGet(guid, sizeof(*guid));
    return S_OK;
}
HRESULT CLSIDFromString(LPCWSTR, GUID *clsid)
{
    memset(clsid, 0, sizeof(*clsid));
    return E_FAIL;
}

// ===========================================================================
// registry stubs
// ===========================================================================
LONG RegOpenKeyExA(HKEY, LPCSTR, DWORD, DWORD, HKEY *) { return ERROR_FILE_NOT_FOUND; }
LONG RegQueryValueExA(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD) { return ERROR_FILE_NOT_FOUND; }
LONG RegSetValueExA(HKEY, LPCSTR, DWORD, DWORD, const BYTE *, DWORD) { return ERROR_ACCESS_DENIED; }
LONG RegCreateKeyExA(HKEY, LPCSTR, DWORD, LPSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, HKEY *, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG RegCloseKey(HKEY) { return ERROR_SUCCESS; }
