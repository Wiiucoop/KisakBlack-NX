// windows.h -- Win32 API compatibility shim for the Nintendo Switch build.
//
// Provides the subset of the Win32 API that KisakBlack actually uses, mapped
// onto libnx/newlib in src/nx/nx_wincompat.cpp. Types use fixed widths (LP64:
// a real `long` is 8 bytes, Win32 LONG must stay 4).
//
// NOTE: the host FS is case-insensitive so this file also satisfies
// #include <Windows.h>. If the tree is ever built on Linux, add a `Windows.h`
// wrapper that includes this file.
#ifndef NX_COMPAT_WINDOWS_H
#define NX_COMPAT_WINDOWS_H

#include "nx_prefix.h"
#include <stdint.h>
#include <stddef.h>

// ===========================================================================
// Base types
// ===========================================================================
typedef int            BOOL;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef uint32_t       DWORD;
typedef uint64_t       DWORD64;
typedef uint64_t       ULONGLONG;
typedef int64_t        LONGLONG;
typedef int32_t        LONG;
typedef uint32_t       ULONG;
typedef int32_t        INT;
typedef uint32_t       UINT;
typedef int16_t        SHORT;
typedef uint16_t       USHORT;
typedef char           CHAR;
typedef unsigned char  UCHAR;
typedef float          FLOAT;
typedef void           VOID;

typedef uint8_t        UINT8;
typedef int8_t         INT8;
typedef uint16_t       UINT16;
typedef int16_t        INT16;
typedef uint32_t       UINT32;
typedef int32_t        INT32;
typedef uint64_t       UINT64;
typedef int64_t        INT64;
typedef int64_t        LONG64;
typedef uint64_t       ULONG64;

typedef intptr_t       INT_PTR;
typedef uintptr_t      UINT_PTR;
typedef intptr_t       LONG_PTR;
typedef uintptr_t      ULONG_PTR;
typedef ULONG_PTR      SIZE_T;
typedef LONG_PTR       SSIZE_T;
typedef ULONG_PTR      DWORD_PTR;

typedef void          *PVOID;
typedef void          *LPVOID;
typedef const void    *LPCVOID;
typedef char          *LPSTR;
typedef char          *PSTR;
typedef const char    *LPCSTR;
typedef const char    *PCSTR;
typedef BYTE          *LPBYTE;
typedef BYTE          *PBYTE;
typedef WORD          *LPWORD;
typedef DWORD         *LPDWORD;
typedef DWORD         *PDWORD;
typedef LONG          *LPLONG;
typedef LONG          *PLONG;
typedef ULONG         *PULONG;
typedef BOOL          *LPBOOL;
typedef UINT          *PUINT;

typedef wchar_t        WCHAR; // 32-bit on GCC; only CubeMapGenLib uses it in kept code
typedef WCHAR         *LPWSTR;
typedef const WCHAR   *LPCWSTR;
typedef WCHAR         *PWSTR;
typedef const WCHAR   *PCWSTR;
typedef CHAR          *LPTSTR;
typedef const CHAR    *LPCTSTR;

typedef void          *HANDLE;
typedef HANDLE        *PHANDLE, *LPHANDLE;

// Real-Windows-style opaque handle tags; the codebase spells types like
// `HWND__ *` directly in places.
#define DECLARE_HANDLE(name) \
    struct name##__ { int unused; }; \
    typedef struct name##__ *name
DECLARE_HANDLE(HWND);
DECLARE_HANDLE(HINSTANCE);
DECLARE_HANDLE(HDC);
DECLARE_HANDLE(HICON);
DECLARE_HANDLE(HCURSOR);
DECLARE_HANDLE(HBRUSH);
DECLARE_HANDLE(HMENU);
DECLARE_HANDLE(HFONT);
DECLARE_HANDLE(HBITMAP);
DECLARE_HANDLE(HGDIOBJ);
DECLARE_HANDLE(HRGN);
DECLARE_HANDLE(HKEY);
DECLARE_HANDLE(HHOOK);
DECLARE_HANDLE(HKL);
DECLARE_HANDLE(HGLOBAL);
DECLARE_HANDLE(HLOCAL);
DECLARE_HANDLE(HRSRC);
DECLARE_HANDLE(HMONITOR);
typedef HINSTANCE HMODULE;

typedef LONG    HRESULT;
typedef WORD    ATOM;
typedef DWORD   COLORREF;
typedef LONG_PTR LRESULT;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;

typedef struct _GUID {
    DWORD Data1;
    WORD  Data2;
    WORD  Data3;
    BYTE  Data4[8];
} GUID;
typedef GUID IID;
typedef const GUID &REFIID;
typedef const GUID &REFGUID;
typedef GUID *LPGUID;

#define WINAPI
#define APIENTRY
#define CALLBACK
#define WINUSERAPI
#define WINBASEAPI
#define PASCAL
#define FAR
#define NEAR
#define CONST const
#define VOID_ void

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

#define MAX_PATH 260
#define INFINITE 0xFFFFFFFFu
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

#define MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | (((WORD)((BYTE)(b))) << 8)))
#define MAKELONG(a, b) ((LONG)(((WORD)(a)) | (((DWORD)((WORD)(b))) << 16)))
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w) ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w) ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

#define UNREFERENCED_PARAMETER(P) (void)(P)
#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif
#define STILL_ACTIVE 259
#define IDC_WAIT ((LPCSTR)32514)

typedef struct tagPOINT { LONG x; LONG y; } POINT, *PPOINT, *LPPOINT;
typedef struct tagRECT  { LONG left; LONG top; LONG right; LONG bottom; } RECT, *PRECT, *LPRECT;
typedef const RECT *LPCRECT;
typedef struct tagSIZE  { LONG cx; LONG cy; } SIZE, *PSIZE;

typedef union _LARGE_INTEGER {
    struct { DWORD LowPart; LONG HighPart; } u;
    struct { DWORD LowPart; LONG HighPart; }; // anonymous (ms-extensions)
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
    struct { DWORD LowPart; DWORD HighPart; };
    ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _SYSTEMTIME {
    WORD wYear, wMonth, wDayOfWeek, wDay;
    WORD wHour, wMinute, wSecond, wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef struct _SECURITY_ATTRIBUTES {
    DWORD  nLength;
    LPVOID lpSecurityDescriptor;
    BOOL   bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

// ===========================================================================
// Errors
// ===========================================================================
#define ERROR_SUCCESS            0
#define NO_ERROR                 0
#define ERROR_FILE_NOT_FOUND     2
#define ERROR_PATH_NOT_FOUND     3
#define ERROR_ACCESS_DENIED      5
#define ERROR_INVALID_HANDLE     6
#define ERROR_NOT_ENOUGH_MEMORY  8
#define ERROR_NO_MORE_FILES      18
#define ERROR_HANDLE_EOF         38
#define ERROR_INVALID_PARAMETER  87
#define ERROR_INSUFFICIENT_BUFFER 122
#define ERROR_ALREADY_EXISTS     183
#define ERROR_IO_PENDING         997

DWORD GetLastError(void);
void  SetLastError(DWORD err);

#define S_OK          ((HRESULT)0)
#define S_FALSE       ((HRESULT)1)
#define E_FAIL        ((HRESULT)0x80004005)
#define E_OUTOFMEMORY ((HRESULT)0x8007000E)
#define E_INVALIDARG  ((HRESULT)0x80070057)
#define E_NOTIMPL     ((HRESULT)0x80004001)
#define E_NOINTERFACE ((HRESULT)0x80004002)
#define E_POINTER     ((HRESULT)0x80004003)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr)    (((HRESULT)(hr)) < 0)

// COM-style method decl macros (used by the XAudio2/XAPO headers)
#define STDMETHODCALLTYPE
#define STDMETHODVCALLTYPE
#define STDAPICALLTYPE
#define STDMETHOD(method)        virtual HRESULT method
#define STDMETHOD_(type, method) virtual type method
#define THIS_
#define THIS void
#define PURE = 0
#define DECLARE_INTERFACE(iface) struct iface
#define DECLARE_INTERFACE_(iface, base) struct iface : public base

#ifdef __cplusplus
class IUnknown {
public:
    virtual HRESULT QueryInterface(REFIID riid, void **object) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
};
#endif

// ===========================================================================
// Interlocked / atomics
// ===========================================================================
static inline LONG InterlockedIncrement(volatile LONG *p)  { return __atomic_add_fetch(p, 1, __ATOMIC_SEQ_CST); }
static inline LONG InterlockedDecrement(volatile LONG *p)  { return __atomic_sub_fetch(p, 1, __ATOMIC_SEQ_CST); }
static inline LONG InterlockedExchange(volatile LONG *p, LONG v) { return __atomic_exchange_n(p, v, __ATOMIC_SEQ_CST); }
static inline LONG InterlockedExchangeAdd(volatile LONG *p, LONG v) { return __atomic_fetch_add(p, v, __ATOMIC_SEQ_CST); }
static inline LONG InterlockedCompareExchange(volatile LONG *p, LONG exchange, LONG comparand)
{
    LONG expected = comparand;
    __atomic_compare_exchange_n(p, &expected, exchange, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}
static inline LONGLONG InterlockedCompareExchange64(volatile LONGLONG *p, LONGLONG exchange, LONGLONG comparand)
{
    LONGLONG expected = comparand;
    __atomic_compare_exchange_n(p, &expected, exchange, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}
static inline PVOID InterlockedExchangePointer(volatile PVOID *p, PVOID v) { return __atomic_exchange_n(p, v, __ATOMIC_SEQ_CST); }
static inline PVOID InterlockedCompareExchangePointer(volatile PVOID *p, PVOID exchange, PVOID comparand)
{
    PVOID expected = comparand;
    __atomic_compare_exchange_n(p, &expected, exchange, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}
static inline LONGLONG InterlockedExchange64(volatile LONGLONG *p, LONGLONG v) { return __atomic_exchange_n(p, v, __ATOMIC_SEQ_CST); }
static inline LONGLONG InterlockedIncrement64(volatile LONGLONG *p) { return __atomic_add_fetch(p, 1, __ATOMIC_SEQ_CST); }

#ifdef __cplusplus
// call sites also spell the argument as (volatile long*) / (volatile unsigned
// int*); Win32 LONG semantics are 32-bit either way on this codebase.
static inline LONG InterlockedIncrement(volatile long *p)  { return InterlockedIncrement((volatile LONG *)p); }
static inline LONG InterlockedDecrement(volatile long *p)  { return InterlockedDecrement((volatile LONG *)p); }
static inline LONG InterlockedExchange(volatile long *p, long v) { return InterlockedExchange((volatile LONG *)p, (LONG)v); }
static inline LONG InterlockedExchangeAdd(volatile long *p, long v) { return InterlockedExchangeAdd((volatile LONG *)p, (LONG)v); }
static inline LONG InterlockedCompareExchange(volatile long *p, long exchange, long comparand)
{
    return InterlockedCompareExchange((volatile LONG *)p, (LONG)exchange, (LONG)comparand);
}
static inline LONG InterlockedIncrement(volatile unsigned int *p)  { return InterlockedIncrement((volatile LONG *)p); }
static inline LONG InterlockedDecrement(volatile unsigned int *p)  { return InterlockedDecrement((volatile LONG *)p); }
static inline LONG InterlockedExchange(volatile unsigned int *p, unsigned int v) { return InterlockedExchange((volatile LONG *)p, (LONG)v); }
static inline LONG InterlockedExchangeAdd(volatile unsigned int *p, unsigned int v) { return InterlockedExchangeAdd((volatile LONG *)p, (LONG)v); }
static inline LONG InterlockedCompareExchange(volatile unsigned int *p, unsigned int exchange, unsigned int comparand)
{
    return InterlockedCompareExchange((volatile LONG *)p, (LONG)exchange, (LONG)comparand);
}
#endif
#define MemoryBarrier() __atomic_thread_fence(__ATOMIC_SEQ_CST)
#define _ReadWriteBarrier() __asm__ __volatile__("" ::: "memory")

// ===========================================================================
// Critical sections / synchronization
// ===========================================================================
typedef struct _RTL_CRITICAL_SECTION {
    // Opaque storage; see nx_wincompat.cpp (libnx RMutex + init flag).
    uint32_t opaque[6];
} CRITICAL_SECTION, *PCRITICAL_SECTION, *LPCRITICAL_SECTION;

void InitializeCriticalSection(LPCRITICAL_SECTION cs);
void DeleteCriticalSection(LPCRITICAL_SECTION cs);
void EnterCriticalSection(LPCRITICAL_SECTION cs);
void LeaveCriticalSection(LPCRITICAL_SECTION cs);
BOOL TryEnterCriticalSection(LPCRITICAL_SECTION cs);

HANDLE CreateEventA(LPSECURITY_ATTRIBUTES sa, BOOL manualReset, BOOL initialState, LPCSTR name);
#define CreateEvent CreateEventA
BOOL SetEvent(HANDLE event);
BOOL ResetEvent(HANDLE event);
BOOL PulseEvent(HANDLE event);
HANDLE CreateSemaphoreA(LPSECURITY_ATTRIBUTES sa, LONG initial, LONG maximum, LPCSTR name);
#define CreateSemaphore CreateSemaphoreA
BOOL ReleaseSemaphore(HANDLE sem, LONG count, LPLONG prev);
HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES sa, BOOL initialOwner, LPCSTR name);
#define CreateMutex CreateMutexA
BOOL ReleaseMutex(HANDLE mutex);

#define WAIT_OBJECT_0  0u
#define WAIT_ABANDONED 0x80u
#define WAIT_TIMEOUT   0x102u
#define WAIT_FAILED    0xFFFFFFFFu

DWORD WaitForSingleObject(HANDLE handle, DWORD ms);
DWORD WaitForMultipleObjects(DWORD count, const HANDLE *handles, BOOL waitAll, DWORD ms);
BOOL  CloseHandle(HANDLE handle);

// ===========================================================================
// Threads
// ===========================================================================
typedef DWORD (WINAPI *LPTHREAD_START_ROUTINE)(LPVOID);
#define CREATE_SUSPENDED 0x4

HANDLE CreateThread(LPSECURITY_ATTRIBUTES sa, SIZE_T stackSize, LPTHREAD_START_ROUTINE start,
                    LPVOID param, DWORD flags, LPDWORD threadId);
DWORD  ResumeThread(HANDLE thread);
DWORD  SuspendThread(HANDLE thread);
BOOL   TerminateThread(HANDLE thread, DWORD exitCode);
void   ExitThread(DWORD exitCode) __attribute__((noreturn));
HANDLE GetCurrentThread(void);
DWORD  GetCurrentThreadId(void);
DWORD  GetCurrentProcessId(void);
HANDLE GetCurrentProcess(void);
BOOL   GetExitCodeThread(HANDLE thread, LPDWORD exitCode);

#define THREAD_PRIORITY_IDLE         (-15)
#define THREAD_PRIORITY_LOWEST       (-2)
#define THREAD_PRIORITY_BELOW_NORMAL (-1)
#define THREAD_PRIORITY_NORMAL       0
#define THREAD_PRIORITY_ABOVE_NORMAL 1
#define THREAD_PRIORITY_HIGHEST      2
#define THREAD_PRIORITY_TIME_CRITICAL 15
BOOL      SetThreadPriority(HANDLE thread, int priority);
int       GetThreadPriority(HANDLE thread);
DWORD_PTR SetThreadAffinityMask(HANDLE thread, DWORD_PTR mask);
DWORD     SetThreadIdealProcessor(HANDLE thread, DWORD processor);

void  Sleep(DWORD ms);
DWORD SleepEx(DWORD ms, BOOL alertable);
BOOL  SwitchToThread(void);

DWORD TlsAlloc(void);
BOOL  TlsFree(DWORD index);
LPVOID TlsGetValue(DWORD index);
BOOL  TlsSetValue(DWORD index, LPVOID value);
#define TLS_OUT_OF_INDEXES 0xFFFFFFFFu

typedef struct _EXCEPTION_POINTERS { void *ExceptionRecord; void *ContextRecord; } EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

typedef struct _LUID { DWORD LowPart; LONG HighPart; } LUID, *PLUID;
typedef struct _LUID_AND_ATTRIBUTES { LUID Luid; DWORD Attributes; } LUID_AND_ATTRIBUTES;
typedef struct _TOKEN_PRIVILEGES {
    DWORD PrivilegeCount;
    LUID_AND_ATTRIBUTES Privileges[1];
} TOKEN_PRIVILEGES, *PTOKEN_PRIVILEGES;
#define SE_PRIVILEGE_ENABLED 0x2u
#define TOKEN_ADJUST_PRIVILEGES 0x20u
#define TOKEN_QUERY 0x8u
BOOL LookupPrivilegeValueA(LPCSTR system, LPCSTR name, PLUID luid);
BOOL AdjustTokenPrivileges(HANDLE token, BOOL disableAll, PTOKEN_PRIVILEGES newState,
                           DWORD bufferLength, PTOKEN_PRIVILEGES previousState, PDWORD returnLength);
typedef LONG (WINAPI *LPTOP_LEVEL_EXCEPTION_FILTER)(EXCEPTION_POINTERS *);
LPTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER filter);
#define EXCEPTION_EXECUTE_HANDLER 1
#define EXCEPTION_CONTINUE_SEARCH 0
#define EXCEPTION_CONTINUE_EXECUTION (-1)
void RaiseException(DWORD code, DWORD flags, DWORD nargs, const ULONG_PTR *args);

// ===========================================================================
// Timing
// ===========================================================================
BOOL QueryPerformanceCounter(LARGE_INTEGER *count);
BOOL QueryPerformanceFrequency(LARGE_INTEGER *freq);
DWORD GetTickCount(void);
ULONGLONG GetTickCount64(void);
void GetSystemTime(LPSYSTEMTIME t);
void GetLocalTime(LPSYSTEMTIME t);
void GetSystemTimeAsFileTime(LPFILETIME t);
BOOL SystemTimeToFileTime(const SYSTEMTIME *st, LPFILETIME ft);
BOOL FileTimeToSystemTime(const FILETIME *ft, LPSYSTEMTIME st);
LONG CompareFileTime(const FILETIME *a, const FILETIME *b);

// mmsystem.h (timeGetTime & friends live here on Windows; game includes windows.h)
DWORD timeGetTime(void);
UINT  timeBeginPeriod(UINT period);
UINT  timeEndPeriod(UINT period);

// ===========================================================================
// Files
// ===========================================================================
#define GENERIC_READ  0x80000000u
#define GENERIC_WRITE 0x40000000u
#define FILE_SHARE_READ   0x1u
#define FILE_SHARE_WRITE  0x2u
#define FILE_SHARE_DELETE 0x4u
#define CREATE_NEW        1
#define CREATE_ALWAYS     2
#define OPEN_EXISTING     3
#define OPEN_ALWAYS       4
#define TRUNCATE_EXISTING 5
#define FILE_ATTRIBUTE_READONLY  0x1u
#define FILE_ATTRIBUTE_HIDDEN    0x2u
#define FILE_ATTRIBUTE_SYSTEM    0x4u
#define FILE_ATTRIBUTE_DIRECTORY 0x10u
#define FILE_ATTRIBUTE_ARCHIVE   0x20u
#define FILE_ATTRIBUTE_NORMAL    0x80u
#define FILE_FLAG_OVERLAPPED     0x40000000u
#define FILE_FLAG_NO_BUFFERING   0x20000000u
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000u
#define INVALID_FILE_ATTRIBUTES  0xFFFFFFFFu
#define INVALID_FILE_SIZE        0xFFFFFFFFu
#define INVALID_SET_FILE_POINTER 0xFFFFFFFFu
#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2

typedef struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union {
        struct { DWORD Offset; DWORD OffsetHigh; };
        PVOID Pointer;
    };
    HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef void (WINAPI *LPOVERLAPPED_COMPLETION_ROUTINE)(DWORD errorCode, DWORD transferred, LPOVERLAPPED ov);
#define WAIT_IO_COMPLETION 0xC0u

HANDLE CreateFileA(LPCSTR name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES sa,
                   DWORD disposition, DWORD flags, HANDLE tmpl);
#define CreateFile CreateFileA
BOOL  ReadFile(HANDLE file, LPVOID buf, DWORD toRead, LPDWORD read, LPOVERLAPPED ov);
BOOL  ReadFileEx(HANDLE file, LPVOID buf, DWORD toRead, LPOVERLAPPED ov,
                 LPOVERLAPPED_COMPLETION_ROUTINE completion);
BOOL  WriteFile(HANDLE file, LPCVOID buf, DWORD toWrite, LPDWORD written, LPOVERLAPPED ov);
BOOL  GetOverlappedResult(HANDLE file, LPOVERLAPPED ov, LPDWORD transferred, BOOL wait);
DWORD SetFilePointer(HANDLE file, LONG dist, PLONG distHigh, DWORD method);
BOOL  SetFilePointerEx(HANDLE file, LARGE_INTEGER dist, PLARGE_INTEGER newPos, DWORD method);
BOOL  SetEndOfFile(HANDLE file);
DWORD GetFileSize(HANDLE file, LPDWORD sizeHigh);
BOOL  GetFileSizeEx(HANDLE file, PLARGE_INTEGER size);
BOOL  FlushFileBuffers(HANDLE file);
BOOL  DeleteFileA(LPCSTR name);
#define DeleteFile DeleteFileA
BOOL  MoveFileA(LPCSTR from, LPCSTR to);
#define MoveFile MoveFileA
BOOL  CopyFileA(LPCSTR from, LPCSTR to, BOOL failIfExists);
#define CopyFile CopyFileA
BOOL  CreateDirectoryA(LPCSTR name, LPSECURITY_ATTRIBUTES sa);
#define CreateDirectory CreateDirectoryA
BOOL  RemoveDirectoryA(LPCSTR name);
DWORD GetFileAttributesA(LPCSTR name);
#define GetFileAttributes GetFileAttributesA
BOOL  SetFileAttributesA(LPCSTR name, DWORD attrs);
DWORD GetCurrentDirectoryA(DWORD len, LPSTR buf);
BOOL  SetCurrentDirectoryA(LPCSTR path);
DWORD GetFullPathNameA(LPCSTR name, DWORD len, LPSTR buf, LPSTR *filePart);
BOOL  GetDiskFreeSpaceA(LPCSTR root, LPDWORD sectorsPerCluster, LPDWORD bytesPerSector,
                        LPDWORD freeClusters, LPDWORD totalClusters);

typedef struct _WIN32_FIND_DATAA {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwReserved0;
    DWORD    dwReserved1;
    CHAR     cFileName[MAX_PATH];
    CHAR     cAlternateFileName[14];
} WIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;
#define WIN32_FIND_DATA WIN32_FIND_DATAA

HANDLE FindFirstFileA(LPCSTR pattern, LPWIN32_FIND_DATAA data);
#define FindFirstFile FindFirstFileA
BOOL   FindNextFileA(HANDLE find, LPWIN32_FIND_DATAA data);
#define FindNextFile FindNextFileA
BOOL   FindClose(HANDLE find);

DWORD GetModuleFileNameA(HMODULE module, LPSTR buf, DWORD len);
#define GetModuleFileName GetModuleFileNameA
HMODULE GetModuleHandleA(LPCSTR name);
#define GetModuleHandle GetModuleHandleA
HMODULE LoadLibraryA(LPCSTR name);
#define LoadLibrary LoadLibraryA
BOOL  FreeLibrary(HMODULE module);
void *GetProcAddress(HMODULE module, LPCSTR name);

// ===========================================================================
// Memory
// ===========================================================================
#define MEM_COMMIT   0x1000u
#define MEM_RESERVE  0x2000u
#define MEM_DECOMMIT 0x4000u
#define MEM_RELEASE  0x8000u
#define MEM_TOP_DOWN 0x100000u
#define PAGE_NOACCESS  0x01u
#define PAGE_READONLY  0x02u
#define PAGE_READWRITE 0x04u
#define PAGE_EXECUTE_READWRITE 0x40u

LPVOID VirtualAlloc(LPVOID address, SIZE_T size, DWORD type, DWORD protect);
BOOL   VirtualFree(LPVOID address, SIZE_T size, DWORD type);
BOOL   VirtualProtect(LPVOID address, SIZE_T size, DWORD newProtect, PDWORD oldProtect);

typedef struct _MEMORY_BASIC_INFORMATION {
    PVOID  BaseAddress;
    PVOID  AllocationBase;
    DWORD  AllocationProtect;
    SIZE_T RegionSize;
    DWORD  State;
    DWORD  Protect;
    DWORD  Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;
SIZE_T VirtualQuery(LPCVOID address, PMEMORY_BASIC_INFORMATION buf, SIZE_T length);

HANDLE GetProcessHeap(void);
LPVOID HeapAlloc(HANDLE heap, DWORD flags, SIZE_T size);
LPVOID HeapReAlloc(HANDLE heap, DWORD flags, LPVOID mem, SIZE_T size);
BOOL   HeapFree(HANDLE heap, DWORD flags, LPVOID mem);
#define HEAP_ZERO_MEMORY 0x8u

#define GMEM_FIXED    0x0
#define GMEM_MOVEABLE 0x2
#define GMEM_ZEROINIT 0x40
#define GPTR (GMEM_FIXED | GMEM_ZEROINIT)
HGLOBAL GlobalAlloc(UINT flags, SIZE_T size);
HGLOBAL GlobalFree(HGLOBAL mem);
LPVOID  GlobalLock(HGLOBAL mem);
BOOL    GlobalUnlock(HGLOBAL mem);
SIZE_T  GlobalSize(HGLOBAL mem);

typedef struct _MEMORYSTATUS {
    DWORD  dwLength;
    DWORD  dwMemoryLoad;
    SIZE_T dwTotalPhys;
    SIZE_T dwAvailPhys;
    SIZE_T dwTotalPageFile;
    SIZE_T dwAvailPageFile;
    SIZE_T dwTotalVirtual;
    SIZE_T dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;
void GlobalMemoryStatus(LPMEMORYSTATUS status);

typedef struct _MEMORYSTATUSEX {
    DWORD     dwLength;
    DWORD     dwMemoryLoad;
    DWORD64   ullTotalPhys;
    DWORD64   ullAvailPhys;
    DWORD64   ullTotalPageFile;
    DWORD64   ullAvailPageFile;
    DWORD64   ullTotalVirtual;
    DWORD64   ullAvailVirtual;
    DWORD64   ullAvailExtendedVirtual;
} MEMORYSTATUSEX, *LPMEMORYSTATUSEX;
BOOL GlobalMemoryStatusEx(LPMEMORYSTATUSEX status);

BOOL IsBadReadPtr(const void *p, UINT_PTR cb);
BOOL IsBadWritePtr(void *p, UINT_PTR cb);
void ZeroMemory_impl(void *p, SIZE_T n);
#define ZeroMemory(p, n) memset((p), 0, (n))
#define CopyMemory(d, s, n) memcpy((d), (s), (n))
#define FillMemory(d, n, v) memset((d), (v), (n))
#define MoveMemory(d, s, n) memmove((d), (s), (n))

// ===========================================================================
// System info / process
// ===========================================================================
typedef struct _SYSTEM_INFO {
    DWORD  dwOemId;
    DWORD  dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD  dwNumberOfProcessors;
    DWORD  dwProcessorType;
    DWORD  dwAllocationGranularity;
    WORD   wProcessorLevel;
    WORD   wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;
void GetSystemInfo(LPSYSTEM_INFO info);

typedef struct _OSVERSIONINFOA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR  szCSDVersion[128];
} OSVERSIONINFOA, *LPOSVERSIONINFOA;
#define OSVERSIONINFO OSVERSIONINFOA
#define VER_PLATFORM_WIN32_NT 2
BOOL GetVersionExA(LPOSVERSIONINFOA info);
#define GetVersionEx GetVersionExA

DWORD GetEnvironmentVariableA(LPCSTR name, LPSTR buf, DWORD len);
BOOL  SetEnvironmentVariableA(LPCSTR name, LPCSTR value);
LPSTR GetCommandLineA(void);
DWORD ExpandEnvironmentStringsA(LPCSTR src, LPSTR dst, DWORD len);

void ExitProcess(UINT code) __attribute__((noreturn));
BOOL TerminateProcess(HANDLE process, UINT code);
UINT SetErrorMode(UINT mode);
BOOL IsDebuggerPresent(void);
void OutputDebugStringA(LPCSTR str);
#define OutputDebugString OutputDebugStringA
void DebugBreak(void);

typedef struct _STARTUPINFOA {
    DWORD  cb;
    LPSTR  lpReserved, lpDesktop, lpTitle;
    DWORD  dwX, dwY, dwXSize, dwYSize, dwXCountChars, dwYCountChars, dwFillAttribute, dwFlags;
    WORD   wShowWindow, cbReserved2;
    LPBYTE lpReserved2;
    HANDLE hStdInput, hStdOutput, hStdError;
} STARTUPINFOA, *LPSTARTUPINFOA;
#define STARTUPINFO STARTUPINFOA
typedef struct _PROCESS_INFORMATION {
    HANDLE hProcess, hThread;
    DWORD  dwProcessId, dwThreadId;
} PROCESS_INFORMATION, *LPPROCESS_INFORMATION;
BOOL CreateProcessA(LPCSTR app, LPSTR cmdline, LPSECURITY_ATTRIBUTES pa, LPSECURITY_ATTRIBUTES ta,
                    BOOL inherit, DWORD flags, LPVOID env, LPCSTR cwd,
                    LPSTARTUPINFOA si, LPPROCESS_INFORMATION pi);
void GetStartupInfoA(LPSTARTUPINFOA si);

// ===========================================================================
// Strings / locale
// ===========================================================================
#define CP_ACP  0
#define CP_UTF8 65001
int MultiByteToWideChar(UINT cp, DWORD flags, LPCSTR src, int srcLen, LPWSTR dst, int dstLen);
int WideCharToMultiByte(UINT cp, DWORD flags, LPCWSTR src, int srcLen, LPSTR dst, int dstLen,
                        LPCSTR defaultChar, LPBOOL usedDefault);
int lstrlenA(LPCSTR s);
LPSTR lstrcpyA(LPSTR dst, LPCSTR src);
int lstrcmpA(LPCSTR a, LPCSTR b);
int lstrcmpiA(LPCSTR a, LPCSTR b);
LPSTR CharLowerA(LPSTR s);
LPSTR CharUpperA(LPSTR s);
DWORD GetUserNameA_shim(LPSTR buf, LPDWORD len);
#define GetUserNameA GetUserNameA_shim

// ===========================================================================
// Window / message subset (all no-op'd or emulated in nx_wincompat.cpp)
// ===========================================================================
typedef LRESULT (CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef LRESULT (CALLBACK *HOOKPROC)(int, WPARAM, LPARAM);

typedef struct tagWNDCLASSA {
    UINT      style;
    WNDPROC   lpfnWndProc;
    int       cbClsExtra, cbWndExtra;
    HINSTANCE hInstance;
    HICON     hIcon;
    HCURSOR   hCursor;
    HBRUSH    hbrBackground;
    LPCSTR    lpszMenuName, lpszClassName;
} WNDCLASSA, *LPWNDCLASSA;
#define WNDCLASS WNDCLASSA

typedef struct tagWNDCLASSEXA {
    UINT      cbSize;
    UINT      style;
    WNDPROC   lpfnWndProc;
    int       cbClsExtra, cbWndExtra;
    HINSTANCE hInstance;
    HICON     hIcon;
    HCURSOR   hCursor;
    HBRUSH    hbrBackground;
    LPCSTR    lpszMenuName, lpszClassName;
    HICON     hIconSm;
} WNDCLASSEXA, *LPWNDCLASSEXA;
#define WNDCLASSEX WNDCLASSEXA

typedef struct tagMSG {
    HWND   hwnd;
    UINT   message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD  time;
    POINT  pt;
} MSG, *PMSG, *LPMSG;

// window messages actually handled by win_wndproc.cpp / rb_backend.cpp
#define WM_NULL           0x0000
#define WM_CREATE         0x0001
#define WM_DESTROY        0x0002
#define WM_MOVE           0x0003
#define WM_SIZE           0x0005
#define WM_ACTIVATE       0x0006
#define WM_SETFOCUS       0x0007
#define WM_KILLFOCUS      0x0008
#define WM_PAINT          0x000F
#define WM_CLOSE          0x0010
#define WM_QUIT           0x0012
#define WM_ERASEBKGND     0x0014
#define WM_SHOWWINDOW     0x0018
#define WM_ACTIVATEAPP    0x001C
#define WM_SETCURSOR      0x0020
#define WM_GETMINMAXINFO  0x0024
#define WM_DISPLAYCHANGE  0x007E
#define WM_KEYDOWN        0x0100
#define WM_KEYUP          0x0101
#define WM_CHAR           0x0102
#define WM_DEADCHAR       0x0103
#define WM_SYSKEYDOWN     0x0104
#define WM_SYSKEYUP       0x0105
#define WM_SYSCHAR        0x0106
#define WM_SYSCOMMAND     0x0112
#define WM_MOUSEMOVE      0x0200
#define WM_LBUTTONDOWN    0x0201
#define WM_LBUTTONUP      0x0202
#define WM_LBUTTONDBLCLK  0x0203
#define WM_RBUTTONDOWN    0x0204
#define WM_RBUTTONUP      0x0205
#define WM_RBUTTONDBLCLK  0x0206
#define WM_MBUTTONDOWN    0x0207
#define WM_MBUTTONUP      0x0208
#define WM_MBUTTONDBLCLK  0x0209
#define WM_MOUSEWHEEL     0x020A
#define WM_XBUTTONDOWN    0x020B
#define WM_XBUTTONUP      0x020C
#define WM_ENTERSIZEMOVE  0x0231
#define WM_EXITSIZEMOVE   0x0232
#define WM_INPUT          0x00FF
#define WM_USER           0x0400

#define WA_INACTIVE    0
#define WA_ACTIVE      1
#define WA_CLICKACTIVE 2
#define SC_KEYMENU     0xF100
#define SC_SCREENSAVE  0xF140
#define SC_MONITORPOWER 0xF170
#define PM_NOREMOVE 0
#define PM_REMOVE   1
#define GWL_STYLE   (-16)
#define GWL_EXSTYLE (-20)
#define GWL_WNDPROC (-4)
#define SW_HIDE 0
#define SW_SHOW 5
#define SW_SHOWNORMAL 1
#define SW_MINIMIZE 6
#define SW_RESTORE 9
#define HWND_TOP      ((HWND)0)
#define HWND_TOPMOST  ((HWND)-1)
#define HWND_NOTOPMOST ((HWND)-2)
#define SWP_NOSIZE   0x1
#define SWP_NOMOVE   0x2
#define SWP_NOZORDER 0x4
#define SWP_SHOWWINDOW 0x40
#define WS_OVERLAPPED  0x00000000u
#define WS_POPUP       0x80000000u
#define WS_VISIBLE     0x10000000u
#define WS_CAPTION     0x00C00000u
#define WS_BORDER      0x00800000u
#define WS_DLGFRAME    0x00400000u
#define WS_SYSMENU     0x00080000u
#define WS_MINIMIZEBOX 0x00020000u
#define WS_MAXIMIZEBOX 0x00010000u
#define WS_THICKFRAME  0x00040000u
#define WS_MINIMIZE    0x20000000u
#define WS_EX_TOPMOST  0x00000008u
#define WS_EX_APPWINDOW 0x00040000u
#define CS_OWNDC   0x0020
#define CS_HREDRAW 0x0002
#define CS_VREDRAW 0x0001
#define CS_DBLCLKS 0x0008
#define IDC_ARROW ((LPCSTR)32512)
#define IDI_APPLICATION ((LPCSTR)32512)
#define COLOR_WINDOW 5
#define COLOR_GRAYTEXT 17
#define BLACK_BRUSH 4
#define MK_LBUTTON 0x0001
#define MK_RBUTTON 0x0002
#define MK_SHIFT   0x0004
#define MK_CONTROL 0x0008
#define MK_MBUTTON 0x0010
#define MK_XBUTTON1 0x0020
#define MK_XBUTTON2 0x0040
#define WHEEL_DELTA 120
#define VK_ESCAPE  0x1B
#define VK_RETURN  0x0D
#define VK_TAB     0x09
#define VK_SHIFT   0x10
#define VK_CONTROL 0x11
#define VK_MENU    0x12
#define VK_SPACE   0x20
#define VK_BACK    0x08
#define VK_CAPITAL 0x14
#define VK_PAUSE   0x13
#define VK_PRIOR   0x21
#define VK_NEXT    0x22
#define VK_END     0x23
#define VK_HOME    0x24
#define VK_LEFT    0x25
#define VK_UP      0x26
#define VK_RIGHT   0x27
#define VK_DOWN    0x28
#define VK_SNAPSHOT 0x2C
#define VK_INSERT  0x2D
#define VK_DELETE  0x2E
#define VK_LWIN    0x5B
#define VK_RWIN    0x5C
#define VK_APPS    0x5D
#define VK_NUMPAD0 0x60
#define VK_MULTIPLY 0x6A
#define VK_ADD     0x6B
#define VK_SEPARATOR 0x6C
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL 0x6E
#define VK_DIVIDE  0x6F
#define VK_F1      0x70
#define VK_F12     0x7B
#define VK_NUMLOCK 0x90
#define VK_SCROLL  0x91
#define VK_LSHIFT  0xA0
#define VK_RSHIFT  0xA1
#define VK_LCONTROL 0xA2
#define VK_RCONTROL 0xA3
#define VK_LMENU   0xA4
#define VK_RMENU   0xA5

typedef struct tagMINMAXINFO {
    POINT ptReserved, ptMaxSize, ptMaxPosition, ptMinTrackSize, ptMaxTrackSize;
} MINMAXINFO, *PMINMAXINFO, *LPMINMAXINFO;

ATOM RegisterClassA(const WNDCLASSA *wc);
ATOM RegisterClassExA(const WNDCLASSEXA *wc);
#define RegisterClass RegisterClassA
#define RegisterClassEx RegisterClassExA
BOOL UnregisterClassA(LPCSTR className, HINSTANCE inst);
HWND CreateWindowExA(DWORD exStyle, LPCSTR className, LPCSTR windowName, DWORD style,
                     int x, int y, int w, int h, HWND parent, HMENU menu, HINSTANCE inst, LPVOID param);
#define CreateWindowEx CreateWindowExA
#define CreateWindowA(cn, wn, st, x, y, w, h, p, m, i, pa) CreateWindowExA(0, cn, wn, st, x, y, w, h, p, m, i, pa)
BOOL DestroyWindow(HWND wnd);
BOOL ShowWindow(HWND wnd, int cmd);
BOOL UpdateWindow(HWND wnd);
BOOL CloseWindow(HWND wnd);
BOOL MoveWindow(HWND wnd, int x, int y, int w, int h, BOOL repaint);
BOOL SetWindowPos(HWND wnd, HWND after, int x, int y, int w, int h, UINT flags);
BOOL GetWindowRect(HWND wnd, LPRECT rect);
BOOL GetClientRect(HWND wnd, LPRECT rect);
BOOL AdjustWindowRect(LPRECT rect, DWORD style, BOOL menu);
BOOL AdjustWindowRectEx(LPRECT rect, DWORD style, BOOL menu, DWORD exStyle);
LONG GetWindowLongA(HWND wnd, int index);
LONG SetWindowLongA(HWND wnd, int index, LONG value);
#define GetWindowLong GetWindowLongA
#define SetWindowLong SetWindowLongA
LONG_PTR GetWindowLongPtrA(HWND wnd, int index);
LONG_PTR SetWindowLongPtrA(HWND wnd, int index, LONG_PTR value);
BOOL SetWindowTextA(HWND wnd, LPCSTR text);
HWND SetFocus(HWND wnd);
HWND GetFocus(void);
HWND GetActiveWindow(void);
HWND SetActiveWindow(HWND wnd);
HWND GetForegroundWindow(void);
BOOL SetForegroundWindow(HWND wnd);
HWND GetDesktopWindow(void);
BOOL IsWindow(HWND wnd);
BOOL IsIconic(HWND wnd);
BOOL BringWindowToTop(HWND wnd);
BOOL EnableWindow(HWND wnd, BOOL enable);
BOOL FlashWindow(HWND wnd, BOOL invert);
HDC  GetDC(HWND wnd);
int  ReleaseDC(HWND wnd, HDC dc);
BOOL InvalidateRect(HWND wnd, const RECT *rect, BOOL erase);

BOOL PeekMessageA(LPMSG msg, HWND wnd, UINT filterMin, UINT filterMax, UINT removeMsg);
#define PeekMessage PeekMessageA
BOOL GetMessageA(LPMSG msg, HWND wnd, UINT filterMin, UINT filterMax);
#define GetMessage GetMessageA
BOOL TranslateMessage(const MSG *msg);
LRESULT DispatchMessageA(const MSG *msg);
#define DispatchMessage DispatchMessageA
BOOL PostMessageA(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
#define PostMessage PostMessageA
LRESULT SendMessageA(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
#define SendMessage SendMessageA
void PostQuitMessage(int exitCode);
LRESULT DefWindowProcA(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
#define DefWindowProc DefWindowProcA
DWORD GetMessageTime(void);

int  ShowCursor(BOOL show);
HCURSOR SetCursor(HCURSOR cur);
HCURSOR GetCursor(void);
HCURSOR LoadCursorA(HINSTANCE inst, LPCSTR name);
#define LoadCursor LoadCursorA
HICON LoadIconA(HINSTANCE inst, LPCSTR name);
#define LoadIcon LoadIconA
BOOL GetCursorPos(LPPOINT pt);
BOOL SetCursorPos(int x, int y);
BOOL ClipCursor(const RECT *rect);
BOOL ScreenToClient(HWND wnd, LPPOINT pt);
BOOL ClientToScreen(HWND wnd, LPPOINT pt);
HWND SetCapture(HWND wnd);
BOOL ReleaseCapture(void);

SHORT GetKeyState(int key);
SHORT GetAsyncKeyState(int key);
BOOL  GetKeyboardState(PBYTE state);
UINT  MapVirtualKeyA(UINT code, UINT mapType);
#define MapVirtualKey MapVirtualKeyA
int   GetKeyNameTextA(LONG lParam, LPSTR str, int size);
HKL   GetKeyboardLayout(DWORD thread);
#define WH_KEYBOARD_LL 13
HHOOK SetWindowsHookExA(int hook, HOOKPROC proc, HINSTANCE mod, DWORD threadId);
BOOL  UnhookWindowsHookEx(HHOOK hook);
LRESULT CallNextHookEx(HHOOK hook, int code, WPARAM wp, LPARAM lp);

int GetSystemMetrics(int index);
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#define SM_REMOTESESSION 0x1000
BOOL SystemParametersInfoA(UINT action, UINT param, PVOID vparam, UINT ini);
#define SystemParametersInfo SystemParametersInfoA
HGDIOBJ GetStockObject(int obj);
// gdi32 device-caps (assertive.cpp desktop restore path)
#define HORZRES 8
#define VERTRES 10
#define BITSPIXEL 12
int  GetDeviceCaps(HDC dc, int index);
BOOL SetDeviceGammaRamp(HDC dc, LPVOID ramp);
BOOL GetDeviceGammaRamp(HDC dc, LPVOID ramp);

#define MB_OK 0x0u
#define MB_OKCANCEL 0x1u
#define MB_YESNO 0x4u
#define MB_YESNOCANCEL 0x3u
#define MB_ICONERROR 0x10u
#define MB_ICONQUESTION 0x20u
#define MB_ICONWARNING 0x30u
#define MB_ICONINFORMATION 0x40u
#define MB_TASKMODAL 0x2000u
#define MB_TOPMOST 0x40000u
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7
int MessageBoxA(HWND wnd, LPCSTR text, LPCSTR caption, UINT type);
#define MessageBox MessageBoxA

// clipboard (Sys_GetClipboardData)
BOOL OpenClipboard(HWND wnd);
BOOL CloseClipboard(void);
HANDLE GetClipboardData(UINT format);
#define CF_TEXT 1

// shell
HINSTANCE ShellExecuteA(HWND wnd, LPCSTR op, LPCSTR file, LPCSTR params, LPCSTR dir, int show);
#define ShellExecute ShellExecuteA

// COM-ish leftovers
HRESULT CoInitialize(LPVOID reserved);
HRESULT CoInitializeEx(LPVOID reserved, DWORD coinit);
void    CoUninitialize(void);
HRESULT CoCreateGuid(GUID *guid);
HRESULT CLSIDFromString(LPCWSTR str, GUID *clsid);

#ifdef __cplusplus
static inline bool operator==(const GUID &a, const GUID &b)
{
    return __builtin_memcmp(&a, &b, sizeof(GUID)) == 0;
}
static inline bool operator!=(const GUID &a, const GUID &b) { return !(a == b); }
#endif

// process/token/etc. (Sys_IsGameProcess, tl_system memory queries)
HANDLE OpenProcess(DWORD access, BOOL inherit, DWORD pid);
BOOL   OpenProcessToken(HANDLE process, DWORD access, PHANDLE token);
BOOL   OpenThreadToken(HANDLE thread, DWORD access, BOOL openAsSelf, PHANDLE token);
BOOL   DuplicateHandle(HANDLE srcProcess, HANDLE src, HANDLE dstProcess, LPHANDLE dst,
                       DWORD access, BOOL inherit, DWORD options);
#define DUPLICATE_SAME_ACCESS 0x2
BOOL   GetProcessAffinityMask(HANDLE process, DWORD_PTR *processMask, DWORD_PTR *systemMask);
BOOL   SetProcessAffinityMask(HANDLE process, DWORD_PTR mask);
#ifdef __cplusplus
static inline BOOL GetProcessAffinityMask(HANDLE process, DWORD *processMask, DWORD *systemMask)
{
    DWORD_PTR p = 0, s = 0;
    BOOL r = GetProcessAffinityMask(process, &p, &s);
    if (processMask) *processMask = (DWORD)p;
    if (systemMask) *systemMask = (DWORD)s;
    return r;
}
#endif
DWORD SetThreadExecutionState(DWORD flags);
#define ES_CONTINUOUS 0x80000000u
#define ES_DISPLAY_REQUIRED 0x2u
#define ES_SYSTEM_REQUIRED 0x1u
UINT RegisterWindowMessageA(LPCSTR name);
#define RegisterWindowMessage RegisterWindowMessageA
BOOL SetPriorityClass(HANDLE process, DWORD priorityClass);
#define HIGH_PRIORITY_CLASS 0x80u
#define NORMAL_PRIORITY_CLASS 0x20u

// wincrypt subset (dwUtils_pc.cpp random bytes)
typedef ULONG_PTR HCRYPTPROV;
#define PROV_RSA_FULL 1
#define CRYPT_VERIFYCONTEXT 0xF0000000u
#define CRYPT_SILENT 0x40u
BOOL CryptAcquireContextA(HCRYPTPROV *prov, LPCSTR container, LPCSTR provider, DWORD provType, DWORD flags);
#define CryptAcquireContext CryptAcquireContextA
BOOL CryptGenRandom(HCRYPTPROV prov, DWORD len, BYTE *buffer);
BOOL CryptReleaseContext(HCRYPTPROV prov, DWORD flags);

// display / monitor stubs
typedef struct _devicemodeA {
    BYTE  dmDeviceName[32];
    WORD  dmSpecVersion, dmDriverVersion, dmSize, dmDriverExtra;
    DWORD dmFields;
    DWORD dmPositionX, dmPositionY, dmDisplayOrientation, dmDisplayFixedOutput;
    short dmColor, dmDuplex, dmYResolution, dmTTOption, dmCollate;
    BYTE  dmFormName[32];
    WORD  dmLogPixels;
    DWORD dmBitsPerPel, dmPelsWidth, dmPelsHeight, dmDisplayFlags, dmDisplayFrequency;
} DEVMODEA, *LPDEVMODEA;
#define DEVMODE DEVMODEA
#define CDS_FULLSCREEN 0x4
#define DISP_CHANGE_SUCCESSFUL 0
LONG ChangeDisplaySettingsA(LPDEVMODEA devMode, DWORD flags);
#define ChangeDisplaySettings ChangeDisplaySettingsA
BOOL EnumDisplaySettingsA(LPCSTR deviceName, DWORD modeNum, LPDEVMODEA devMode);

typedef struct tagMONITORINFO {
    DWORD cbSize;
    RECT  rcMonitor;
    RECT  rcWork;
    DWORD dwFlags;
} MONITORINFO, *LPMONITORINFO;
typedef struct tagMONITORINFOEXA {
    DWORD cbSize;
    RECT  rcMonitor;
    RECT  rcWork;
    DWORD dwFlags;
    CHAR  szDevice[32];
} MONITORINFOEXA, *LPMONITORINFOEXA;
#define MONITOR_DEFAULTTONULL    0
#define MONITOR_DEFAULTTOPRIMARY 1
#define MONITOR_DEFAULTTONEAREST 2
HMONITOR MonitorFromWindow(HWND wnd, DWORD flags);
HMONITOR MonitorFromPoint(POINT pt, DWORD flags);
BOOL GetMonitorInfoA(HMONITOR monitor, LPMONITORINFO info);
#define GetMonitorInfo GetMonitorInfoA
typedef BOOL (CALLBACK *MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);
BOOL EnumDisplayMonitors(HDC dc, const RECT *clip, MONITORENUMPROC proc, LPARAM data);

// gdi / misc window leftovers
HBRUSH CreateSolidBrush(COLORREF color);
HFONT  CreateFontA(int height, int width, int escapement, int orientation, int weight,
                   DWORD italic, DWORD underline, DWORD strikeout, DWORD charset,
                   DWORD outPrecision, DWORD clipPrecision, DWORD quality,
                   DWORD pitchAndFamily, LPCSTR faceName);
BOOL   DeleteObject(HGDIOBJ obj);
HANDLE LoadImageA(HINSTANCE inst, LPCSTR name, UINT type, int cx, int cy, UINT load);
int    GetWindowTextA(HWND wnd, LPSTR text, int maxCount);
typedef BOOL (CALLBACK *WNDENUMPROC)(HWND, LPARAM);
BOOL   EnumThreadWindows(DWORD threadId, WNDENUMPROC fn, LPARAM lp);
LRESULT CallWindowProcA(WNDPROC prev, HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
BOOL   SetClipboardData(UINT format, HANDLE mem);
BOOL   EmptyClipboard(void);
DWORD  FormatMessageA(DWORD flags, LPCVOID source, DWORD messageId, DWORD languageId,
                      LPSTR buffer, DWORD size, va_list *args);
#define FORMAT_MESSAGE_FROM_SYSTEM 0x1000u
#define FORMAT_MESSAGE_IGNORE_INSERTS 0x200u
int MessageBoxW(HWND wnd, const WCHAR *text, const WCHAR *caption, UINT type);

// registry stubs (nothing real uses them; return failure)
#define HKEY_LOCAL_MACHINE ((HKEY)(uintptr_t)0x80000002u)
#define HKEY_CURRENT_USER  ((HKEY)(uintptr_t)0x80000001u)
#define KEY_READ  0x20019
#define KEY_WRITE 0x20006
LONG RegOpenKeyExA(HKEY key, LPCSTR subKey, DWORD options, DWORD sam, HKEY *result);
#define RegOpenKeyEx RegOpenKeyExA
LONG RegQueryValueExA(HKEY key, LPCSTR name, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD size);
#define RegQueryValueEx RegQueryValueExA
LONG RegSetValueExA(HKEY key, LPCSTR name, DWORD reserved, DWORD type, const BYTE *data, DWORD size);
LONG RegCreateKeyExA(HKEY key, LPCSTR subKey, DWORD reserved, LPSTR cls, DWORD options,
                     DWORD sam, LPSECURITY_ATTRIBUTES sa, HKEY *result, LPDWORD disposition);
LONG RegCloseKey(HKEY key);

// WinMain is defined by the game (src/win32/win_main.cpp) and invoked by
// src/nx/nx_main.cpp.
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

// windows.h without WIN32_LEAN_AND_MEAN drags in winsock on real Windows and
// win_net.cpp relies on that.
#include "winsock.h"

#endif // NX_COMPAT_WINDOWS_H
