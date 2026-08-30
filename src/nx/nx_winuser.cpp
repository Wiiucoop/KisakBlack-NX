// nx_winuser.cpp -- user32/gdi32/shell32 implementation for the Switch
// build. There is no window system: window handles are a single fake window
// record, the message queue is always empty, and the registered wndproc is
// invoked directly for the window-lifecycle messages the game's bookkeeping
// depends on (WM_CREATE / WM_ACTIVATE / WM_SETFOCUS).
#include <windows.h>

#include <switch.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ===========================================================================
// window / class bookkeeping
// ===========================================================================
struct NxWindow {
    int x, y, w, h;
    WNDPROC proc;
    char title[128];
    bool inUse;
};

static NxWindow s_window; // the game creates exactly one real window
static WNDPROC s_classProc[8];
static char s_className[8][64];
static int s_classCount;
static HWND s_activeWindow;
static int s_cursorShow = 1;
static POINT s_cursorPos = { 640, 360 };

static NxWindow *nxWnd(HWND w)
{
    return (NxWindow *)w == &s_window && s_window.inUse ? &s_window : NULL;
}

ATOM RegisterClassA(const WNDCLASSA *wc)
{
    if (s_classCount < 8 && wc) {
        s_classProc[s_classCount] = wc->lpfnWndProc;
        strncpy(s_className[s_classCount], wc->lpszClassName ? wc->lpszClassName : "", 63);
        s_classCount++;
    }
    return (ATOM)s_classCount;
}

ATOM RegisterClassExA(const WNDCLASSEXA *wc)
{
    if (s_classCount < 8 && wc) {
        s_classProc[s_classCount] = wc->lpfnWndProc;
        strncpy(s_className[s_classCount], wc->lpszClassName ? wc->lpszClassName : "", 63);
        s_classCount++;
    }
    return (ATOM)s_classCount;
}

BOOL UnregisterClassA(LPCSTR, HINSTANCE) { return TRUE; }

HWND CreateWindowExA(DWORD, LPCSTR className, LPCSTR windowName, DWORD,
                     int x, int y, int w, int h, HWND, HMENU, HINSTANCE, LPVOID)
{
    s_window.inUse = true;
    s_window.x = x;
    s_window.y = y;
    s_window.w = w > 0 ? w : 1280;
    s_window.h = h > 0 ? h : 720;
    s_window.proc = NULL;
    strncpy(s_window.title, windowName ? windowName : "", sizeof(s_window.title) - 1);
    for (int i = 0; i < s_classCount; ++i) {
        if (className && !strcmp(s_className[i], className)) {
            s_window.proc = s_classProc[i];
            break;
        }
    }
    HWND hwnd = (HWND)&s_window;
    s_activeWindow = hwnd;
    // replay the lifecycle messages the game's wndproc bookkeeping expects
    if (s_window.proc) {
        s_window.proc(hwnd, WM_CREATE, 0, 0);
        s_window.proc(hwnd, WM_ACTIVATE, WA_ACTIVE, 0);
        s_window.proc(hwnd, WM_SETFOCUS, 0, 0);
    }
    return hwnd;
}

BOOL DestroyWindow(HWND wnd)
{
    NxWindow *w = nxWnd(wnd);
    if (w) {
        if (w->proc) w->proc(wnd, WM_DESTROY, 0, 0);
        w->inUse = false;
        s_activeWindow = NULL;
    }
    return TRUE;
}

BOOL ShowWindow(HWND, int) { return TRUE; }
BOOL UpdateWindow(HWND) { return TRUE; }
BOOL CloseWindow(HWND) { return TRUE; }

BOOL MoveWindow(HWND wnd, int x, int y, int w, int h, BOOL)
{
    NxWindow *win = nxWnd(wnd);
    if (win) {
        win->x = x;
        win->y = y;
        win->w = w;
        win->h = h;
    }
    return TRUE;
}

BOOL SetWindowPos(HWND wnd, HWND, int x, int y, int w, int h, UINT flags)
{
    NxWindow *win = nxWnd(wnd);
    if (win) {
        if (!(flags & SWP_NOMOVE)) {
            win->x = x;
            win->y = y;
        }
        if (!(flags & SWP_NOSIZE) && w > 0 && h > 0) {
            win->w = w;
            win->h = h;
        }
    }
    return TRUE;
}

BOOL GetWindowRect(HWND wnd, LPRECT rect)
{
    NxWindow *w = nxWnd(wnd);
    rect->left = w ? w->x : 0;
    rect->top = w ? w->y : 0;
    rect->right = rect->left + (w ? w->w : 1280);
    rect->bottom = rect->top + (w ? w->h : 720);
    return TRUE;
}

BOOL GetClientRect(HWND wnd, LPRECT rect)
{
    NxWindow *w = nxWnd(wnd);
    rect->left = 0;
    rect->top = 0;
    rect->right = w ? w->w : 1280;
    rect->bottom = w ? w->h : 720;
    return TRUE;
}

BOOL AdjustWindowRect(LPRECT, DWORD, BOOL) { return TRUE; }
BOOL AdjustWindowRectEx(LPRECT, DWORD, BOOL, DWORD) { return TRUE; }
LONG GetWindowLongA(HWND, int) { return 0; }
LONG SetWindowLongA(HWND, int, LONG value) { return value; }
LONG_PTR GetWindowLongPtrA(HWND, int) { return 0; }
LONG_PTR SetWindowLongPtrA(HWND, int, LONG_PTR value) { return value; }

BOOL SetWindowTextA(HWND wnd, LPCSTR text)
{
    NxWindow *w = nxWnd(wnd);
    if (w && text)
        strncpy(w->title, text, sizeof(w->title) - 1);
    return TRUE;
}

int GetWindowTextA(HWND wnd, LPSTR text, int maxCount)
{
    NxWindow *w = nxWnd(wnd);
    if (!w || !text || maxCount <= 0) return 0;
    strncpy(text, w->title, (size_t)maxCount - 1);
    text[maxCount - 1] = 0;
    return (int)strlen(text);
}

HWND SetFocus(HWND wnd) { HWND prev = s_activeWindow; s_activeWindow = wnd; return prev; }
HWND GetFocus(void) { return s_activeWindow; }
HWND GetActiveWindow(void) { return s_activeWindow; }
HWND SetActiveWindow(HWND wnd) { HWND prev = s_activeWindow; s_activeWindow = wnd; return prev; }
HWND GetForegroundWindow(void) { return s_activeWindow; }
BOOL SetForegroundWindow(HWND) { return TRUE; }
HWND GetDesktopWindow(void) { return (HWND)&s_window; }
BOOL IsWindow(HWND wnd) { return nxWnd(wnd) != NULL; }
BOOL IsIconic(HWND) { return FALSE; }
BOOL BringWindowToTop(HWND) { return TRUE; }
BOOL EnableWindow(HWND, BOOL) { return TRUE; }
BOOL FlashWindow(HWND, BOOL) { return TRUE; }
HDC GetDC(HWND) { return (HDC)(uintptr_t)1; }
int ReleaseDC(HWND, HDC) { return 1; }
BOOL InvalidateRect(HWND, const RECT *, BOOL) { return TRUE; }
BOOL EnumThreadWindows(DWORD, WNDENUMPROC, LPARAM) { return TRUE; }
LRESULT CallWindowProcA(WNDPROC prev, HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    return prev ? prev(wnd, msg, wp, lp) : 0;
}

// ===========================================================================
// message pump: no messages ever arrive; GetMessage reports WM_QUIT so any
// modal Windows-style loop (Sys_Error) falls through and exits.
// ===========================================================================
BOOL PeekMessageA(LPMSG msg, HWND, UINT, UINT, UINT)
{
    if (msg) memset(msg, 0, sizeof(*msg));
    return FALSE;
}

BOOL GetMessageA(LPMSG msg, HWND, UINT, UINT)
{
    if (msg) memset(msg, 0, sizeof(*msg));
    svcSleepThread(10000000ll); // 10ms; avoid a hot spin if called in a loop
    return FALSE;               // "WM_QUIT"
}

BOOL TranslateMessage(const MSG *) { return FALSE; }
LRESULT DispatchMessageA(const MSG *) { return 0; }
BOOL PostMessageA(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    NxWindow *w = nxWnd(wnd);
    if (w && w->proc)
        w->proc(wnd, msg, wp, lp);
    return TRUE;
}
LRESULT SendMessageA(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    NxWindow *w = nxWnd(wnd);
    return (w && w->proc) ? w->proc(wnd, msg, wp, lp) : 0;
}
void PostQuitMessage(int) {}
LRESULT DefWindowProcA(HWND, UINT, WPARAM, LPARAM) { return 0; }
DWORD GetMessageTime(void) { return timeGetTime(); }

// ===========================================================================
// cursor / keyboard
// ===========================================================================
int ShowCursor(BOOL show)
{
    s_cursorShow += show ? 1 : -1;
    return s_cursorShow;
}
HCURSOR SetCursor(HCURSOR cur) { return cur; }
HCURSOR GetCursor(void) { return NULL; }
HCURSOR LoadCursorA(HINSTANCE, LPCSTR) { return (HCURSOR)(uintptr_t)1; }
HICON LoadIconA(HINSTANCE, LPCSTR) { return (HICON)(uintptr_t)1; }
BOOL GetCursorPos(LPPOINT pt)
{
    if (pt) *pt = s_cursorPos;
    return TRUE;
}
BOOL SetCursorPos(int x, int y)
{
    s_cursorPos.x = x;
    s_cursorPos.y = y;
    return TRUE;
}
BOOL ClipCursor(const RECT *) { return TRUE; }
BOOL ScreenToClient(HWND, LPPOINT) { return TRUE; }
BOOL ClientToScreen(HWND, LPPOINT) { return TRUE; }
HWND SetCapture(HWND wnd) { return wnd; }
BOOL ReleaseCapture(void) { return TRUE; }

SHORT GetKeyState(int) { return 0; }
SHORT GetAsyncKeyState(int) { return 0; }
BOOL GetKeyboardState(PBYTE state)
{
    if (state) memset(state, 0, 256);
    return TRUE;
}
UINT MapVirtualKeyA(UINT, UINT) { return 0; }
int GetKeyNameTextA(LONG, LPSTR str, int size)
{
    if (str && size > 0) str[0] = 0;
    return 0;
}
HKL GetKeyboardLayout(DWORD) { return (HKL)(uintptr_t)0x409; }
HHOOK SetWindowsHookExA(int, HOOKPROC, HINSTANCE, DWORD) { return NULL; }
BOOL UnhookWindowsHookEx(HHOOK) { return TRUE; }
LRESULT CallNextHookEx(HHOOK, int, WPARAM, LPARAM) { return 0; }

// ===========================================================================
// system metrics / monitors / display
// ===========================================================================
int GetSystemMetrics(int index)
{
    switch (index) {
    case SM_CXSCREEN: return 1280;
    case SM_CYSCREEN: return 720;
    default: return 0; // includes SM_REMOTESESSION
    }
}

BOOL SystemParametersInfoA(UINT, UINT, PVOID, UINT) { return TRUE; }
HGDIOBJ GetStockObject(int) { return (HGDIOBJ)(uintptr_t)1; }

int GetDeviceCaps(HDC, int index)
{
    switch (index) {
    case HORZRES: return 1280;
    case VERTRES: return 720;
    case BITSPIXEL: return 32;
    default: return 0;
    }
}

BOOL SetDeviceGammaRamp(HDC, LPVOID) { return TRUE; }
BOOL GetDeviceGammaRamp(HDC, LPVOID) { return FALSE; }

HMONITOR MonitorFromWindow(HWND, DWORD) { return (HMONITOR)(uintptr_t)1; }
HMONITOR MonitorFromPoint(POINT, DWORD) { return (HMONITOR)(uintptr_t)1; }

BOOL GetMonitorInfoA(HMONITOR, LPMONITORINFO info)
{
    if (info) {
        info->rcMonitor.left = 0;
        info->rcMonitor.top = 0;
        info->rcMonitor.right = 1280;
        info->rcMonitor.bottom = 720;
        info->rcWork = info->rcMonitor;
        info->dwFlags = 1; // MONITORINFOF_PRIMARY
    }
    return TRUE;
}

BOOL EnumDisplayMonitors(HDC, const RECT *, MONITORENUMPROC proc, LPARAM data)
{
    if (proc) {
        RECT r = { 0, 0, 1280, 720 };
        proc((HMONITOR)(uintptr_t)1, NULL, &r, data);
    }
    return TRUE;
}

LONG ChangeDisplaySettingsA(LPDEVMODEA, DWORD) { return DISP_CHANGE_SUCCESSFUL; }

BOOL EnumDisplaySettingsA(LPCSTR, DWORD modeNum, LPDEVMODEA devMode)
{
    static const int modes[][3] = { { 1280, 720, 60 }, { 1920, 1080, 60 } };
    if (modeNum >= 2 || !devMode) return FALSE;
    memset(devMode, 0, sizeof(*devMode));
    devMode->dmPelsWidth = (DWORD)modes[modeNum][0];
    devMode->dmPelsHeight = (DWORD)modes[modeNum][1];
    devMode->dmDisplayFrequency = (DWORD)modes[modeNum][2];
    devMode->dmBitsPerPel = 32;
    return TRUE;
}

// ===========================================================================
// gdi objects
// ===========================================================================
HBRUSH CreateSolidBrush(COLORREF) { return (HBRUSH)(uintptr_t)1; }
HFONT CreateFontA(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD,
                  DWORD, DWORD, DWORD, DWORD, LPCSTR)
{
    return (HFONT)(uintptr_t)1;
}
BOOL DeleteObject(HGDIOBJ) { return TRUE; }
HANDLE LoadImageA(HINSTANCE, LPCSTR, UINT, int, int, UINT) { return NULL; }

// ===========================================================================
// dialogs / clipboard / shell / misc
// ===========================================================================
int MessageBoxA(HWND, LPCSTR text, LPCSTR caption, UINT type)
{
    fprintf(stderr, "[MessageBox] %s: %s\n", caption ? caption : "", text ? text : "");
    // There is nobody to click a button, so always answer affirmatively.
    // Several prompts ("low memory -- run anyway?") exit the process on a
    // negative answer, so IDNO here would silently kill the game.
    switch (type & 0xF) {
    case 3: // MB_YESNOCANCEL
    case 4: // MB_YESNO
        return IDYES;
    default:
        return IDOK;
    }
}

int MessageBoxW(HWND, const WCHAR *text, const WCHAR *caption, UINT type)
{
    char t[256], c[128];
    unsigned i = 0;
    for (; text && text[i] && i < sizeof(t) - 1; ++i) t[i] = (char)(text[i] < 128 ? text[i] : '?');
    t[i] = 0;
    i = 0;
    for (; caption && caption[i] && i < sizeof(c) - 1; ++i) c[i] = (char)(caption[i] < 128 ? caption[i] : '?');
    c[i] = 0;
    return MessageBoxA(NULL, t, c, type);
}

BOOL OpenClipboard(HWND) { return FALSE; }
BOOL CloseClipboard(void) { return TRUE; }
HANDLE GetClipboardData(UINT) { return NULL; }
BOOL SetClipboardData(UINT, HANDLE) { return FALSE; }
BOOL EmptyClipboard(void) { return TRUE; }
SIZE_T GlobalSize(HGLOBAL) { return 0; }

HINSTANCE ShellExecuteA(HWND, LPCSTR op, LPCSTR file, LPCSTR, LPCSTR, int)
{
    fprintf(stderr, "[ShellExecute] %s %s\n", op ? op : "", file ? file : "");
    return (HINSTANCE)(uintptr_t)33; // >32 == success
}

UINT RegisterWindowMessageA(LPCSTR) { return WM_USER + 0x100; }
BOOL SetPriorityClass(HANDLE, DWORD) { return TRUE; }
DWORD SetThreadExecutionState(DWORD) { return 0; }
BOOL LookupPrivilegeValueA(LPCSTR, LPCSTR, PLUID) { return FALSE; }
BOOL AdjustTokenPrivileges(HANDLE, BOOL, PTOKEN_PRIVILEGES, DWORD, PTOKEN_PRIVILEGES, PDWORD) { return FALSE; }

// ===========================================================================
// pread/pwrite (missing from devkitA64 newlib; used by nx_wincompat)
// ===========================================================================
extern "C" ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
    off_t cur = lseek(fd, 0, SEEK_CUR);
    if (cur < 0) return -1;
    if (lseek(fd, offset, SEEK_SET) < 0) return -1;
    ssize_t n = read(fd, buf, count);
    lseek(fd, cur, SEEK_SET);
    return n;
}

extern "C" ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
    off_t cur = lseek(fd, 0, SEEK_CUR);
    if (cur < 0) return -1;
    if (lseek(fd, offset, SEEK_SET) < 0) return -1;
    ssize_t n = write(fd, buf, count);
    lseek(fd, cur, SEEK_SET);
    return n;
}
