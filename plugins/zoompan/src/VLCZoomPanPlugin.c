#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <wchar.h>

/* VLC 3.0.21's Windows thread header defines an inline vlc_poll() wrapper
 * around poll(), but llvm-mingw does not provide poll.h. This plugin never
 * calls vlc_poll(); the forward declaration below is only to make that
 * unused inline wrapper parse correctly without modifying VLC's headers. */
struct pollfd;
int poll(struct pollfd *fds, unsigned nfds, int timeout);

#define __PLUGIN__ 1
#define MODULE_STRING "zoompan"
#define VLC_MODULE_LICENSE VLC_LICENSE_LGPL_2_1_PLUS
#include <vlc_common.h>
#include <vlc_plugin.h>

static int Open(vlc_object_t *);
static void Close(vlc_object_t *);

static HANDLE g_stop = NULL;
static HANDLE g_thread = NULL;
static DWORD g_pid = 0;

vlc_module_begin()
    set_shortname("ZoomPan")
    set_description("Native zoom/pan mouse controls")
    set_capability("interface", 0)
    add_shortcut("zoompan")
    set_callbacks(Open, Close)
vlc_module_end()

static HWND g_video = NULL;
static HWND g_parent = NULL;
static HWND g_top = NULL;

static BOOL is_our_window(HWND hwnd)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    return pid == g_pid;
}

static BOOL CALLBACK find_top_cb(HWND hwnd, LPARAM lp)
{
    (void)lp;

    if (!is_our_window(hwnd))
        return TRUE;

    if (GetWindow(hwnd, GW_OWNER) == NULL && IsWindowVisible(hwnd)) {
        g_top = hwnd;
        return FALSE;
    }

    return TRUE;
}

static BOOL CALLBACK find_video_cb(HWND hwnd, LPARAM lp)
{
    (void)lp;

    if (!is_our_window(hwnd))
        return TRUE;

    wchar_t cls[256];
    if (GetClassNameW(hwnd, cls, 256) <= 0)
        return TRUE;

    if (wcsncmp(cls, L"VLC video output", 16) == 0) {
        g_video = hwnd;
        g_parent = GetParent(hwnd);
        if (!g_parent)
            g_parent = g_top;
        return FALSE;
    }

    return TRUE;
}

static void resolve_windows(void)
{
    if (g_video && IsWindow(g_video) && g_parent && IsWindow(g_parent))
        return;

    g_video = NULL;
    g_parent = NULL;
    g_top = NULL;

    EnumWindows(find_top_cb, 0);
    if (!g_top)
        return;

    wchar_t cls[256];
    if (GetClassNameW(g_top, cls, 256) > 0 &&
        wcsncmp(cls, L"VLC video output", 16) == 0) {
        g_video = g_top;
        g_parent = GetParent(g_top);
        if (!g_parent)
            g_parent = g_top;
        return;
    }

    EnumChildWindows(g_top, find_video_cb, 0);
}

static BOOL foreground_is_ours(void)
{
    HWND fg = GetForegroundWindow();
    if (!fg)
        return FALSE;

    DWORD pid = 0;
    GetWindowThreadProcessId(fg, &pid);
    return pid == g_pid;
}

static BOOL window_is_descendant_or_same(HWND hwnd, HWND ancestor)
{
    HWND cur = hwnd;

    for (int i = 0; cur && i < 64; ++i) {
        if (cur == ancestor)
            return TRUE;
        cur = GetParent(cur);
    }

    return FALSE;
}

static BOOL get_geometry(int *x, int *y, int *vw, int *vh, int *pw, int *ph)
{
    if (!g_video || !g_parent || !IsWindow(g_video) || !IsWindow(g_parent))
        return FALSE;

    RECT vr, pr;
    if (!GetWindowRect(g_video, &vr))
        return FALSE;
    if (!GetClientRect(g_parent, &pr))
        return FALSE;

    POINT pt = { vr.left, vr.top };
    if (!ScreenToClient(g_parent, &pt))
        return FALSE;

    *x = pt.x;
    *y = pt.y;
    *vw = vr.right - vr.left;
    *vh = vr.bottom - vr.top;
    *pw = pr.right - pr.left;
    *ph = pr.bottom - pr.top;

    return TRUE;
}

static int clamp_axis(int value, int child, int parent)
{
    if (child <= parent)
        return (parent - child) / 2;

    int minimum = parent - child;

    if (value < minimum)
        return minimum;
    if (value > 0)
        return 0;

    return value;
}

static void send_zoom_original(void)
{
    if (!foreground_is_ours())
        return;

    /*
     * The installer maps Ctrl+Alt+0 to key-zoom-original.
     * This sets VLC's scale factor to exactly 1.0 without toggling autoscale.
     */
    INPUT in[6];
    ZeroMemory(in, sizeof(in));

    in[0].type = INPUT_KEYBOARD;
    in[0].ki.wVk = VK_CONTROL;

    in[1].type = INPUT_KEYBOARD;
    in[1].ki.wVk = VK_MENU;

    in[2].type = INPUT_KEYBOARD;
    in[2].ki.wVk = '0';

    in[3].type = INPUT_KEYBOARD;
    in[3].ki.wVk = '0';
    in[3].ki.dwFlags = KEYEVENTF_KEYUP;

    in[4].type = INPUT_KEYBOARD;
    in[4].ki.wVk = VK_MENU;
    in[4].ki.dwFlags = KEYEVENTF_KEYUP;

    in[5].type = INPUT_KEYBOARD;
    in[5].ki.wVk = VK_CONTROL;
    in[5].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(6, in, sizeof(INPUT));
}

static DWORD WINAPI zoompan_thread(LPVOID opaque)
{
    (void)opaque;

    BOOL left_prev = FALSE;
    BOOL middle_prev = FALSE;
    BOOL dragging = FALSE;
    BOOL pan_active = FALSE;
    BOOL fit_active = FALSE;

    POINT drag_cursor = {0, 0};
    int drag_start_x = 0;
    int drag_start_y = 0;
    int wanted_x = 0;
    int wanted_y = 0;

    int fit_ratio_w = 0;
    int fit_ratio_h = 0;

    ULONGLONG last_middle = 0;
    POINT last_middle_pt = {0, 0};

    const UINT dbl_ms = GetDoubleClickTime();
    const int dbl_x = max(4, GetSystemMetrics(SM_CXDOUBLECLK));
    const int dbl_y = max(4, GetSystemMetrics(SM_CYDOUBLECLK));

    ULONGLONG last_resolve = 0;

    while (WaitForSingleObject(g_stop, 10) == WAIT_TIMEOUT) {
        ULONGLONG now = GetTickCount64();

        if (!g_video || !IsWindow(g_video) || now - last_resolve > 500) {
            resolve_windows();
            last_resolve = now;
        }

        BOOL left = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        BOOL middle = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
        BOOL control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

        if (fit_active && control)
            fit_active = FALSE;

        POINT cursor;
        GetCursorPos(&cursor);

        BOOL ours_fg = foreground_is_ours();

        if (ours_fg && left && !left_prev && g_video && g_parent) {
            HWND under = WindowFromPoint(cursor);

            int x, y, vw, vh, pw, ph;
            if (get_geometry(&x, &y, &vw, &vh, &pw, &ph) &&
                window_is_descendant_or_same(under, g_video) &&
                (vw > pw || vh > ph)) {
                dragging = TRUE;
                pan_active = TRUE;
                fit_active = FALSE;

                drag_cursor = cursor;
                drag_start_x = x;
                drag_start_y = y;
                wanted_x = x;
                wanted_y = y;
            }
        }

        if (!left && left_prev)
            dragging = FALSE;

        if (ours_fg && dragging && g_video && g_parent) {
            int x, y, vw, vh, pw, ph;

            if (get_geometry(&x, &y, &vw, &vh, &pw, &ph)) {
                wanted_x = clamp_axis(
                    drag_start_x + (cursor.x - drag_cursor.x), vw, pw);
                wanted_y = clamp_axis(
                    drag_start_y + (cursor.y - drag_cursor.y), vh, ph);
            }
        }

        if (fit_active && g_video && g_parent && IsWindow(g_video) &&
            fit_ratio_w > 0 && fit_ratio_h > 0) {
            int x, y, vw, vh, pw, ph;

            if (get_geometry(&x, &y, &vw, &vh, &pw, &ph) &&
                pw > 0 && ph > 0) {
                int fw = pw;
                int fh = (int)(((long long)pw * fit_ratio_h) / fit_ratio_w);

                if (fh > ph) {
                    fh = ph;
                    fw = (int)(((long long)ph * fit_ratio_w) / fit_ratio_h);
                }

                if (fw < 1)
                    fw = 1;
                if (fh < 1)
                    fh = 1;

                SetWindowPos(
                    g_video,
                    NULL,
                    (pw - fw) / 2,
                    (ph - fh) / 2,
                    fw,
                    fh,
                    SWP_NOZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);
            }
        }
        else if (pan_active && g_video && g_parent && IsWindow(g_video)) {
            int x, y, vw, vh, pw, ph;

            if (get_geometry(&x, &y, &vw, &vh, &pw, &ph)) {
                if (vw <= pw && vh <= ph) {
                    pan_active = FALSE;
                    dragging = FALSE;
                }
                else {
                    wanted_x = clamp_axis(wanted_x, vw, pw);
                    wanted_y = clamp_axis(wanted_y, vh, ph);

                    SetWindowPos(
                        g_video,
                        NULL,
                        wanted_x,
                        wanted_y,
                        0,
                        0,
                        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
                            SWP_ASYNCWINDOWPOS);
                }
            }
        }

        if (ours_fg && middle && !middle_prev) {
            BOOL dbl = FALSE;

            if (last_middle != 0 && now - last_middle <= dbl_ms) {
                int dx = cursor.x - last_middle_pt.x;
                int dy = cursor.y - last_middle_pt.y;

                if (abs(dx) <= dbl_x && abs(dy) <= dbl_y)
                    dbl = TRUE;
            }

            last_middle = now;
            last_middle_pt = cursor;

            if (dbl) {
                dragging = FALSE;
                pan_active = FALSE;

                int x, y, vw, vh, pw, ph;
                BOOL have_ratio = FALSE;

                if (get_geometry(&x, &y, &vw, &vh, &pw, &ph) &&
                    vw > 0 && vh > 0) {
                    fit_ratio_w = vw;
                    fit_ratio_h = vh;
                    have_ratio = TRUE;
                }

                send_zoom_original();
                Sleep(30);
                fit_active = have_ratio;

                last_middle = 0;
            }
        }

        left_prev = left;
        middle_prev = middle;
    }

    return 0;
}

static int Open(vlc_object_t *obj)
{
    (void)obj;

    if (g_thread != NULL)
        return VLC_SUCCESS;

    g_pid = GetCurrentProcessId();

    g_stop = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!g_stop)
        return VLC_EGENERIC;

    g_thread = CreateThread(NULL, 0, zoompan_thread, NULL, 0, NULL);
    if (!g_thread) {
        CloseHandle(g_stop);
        g_stop = NULL;
        return VLC_EGENERIC;
    }

    return VLC_SUCCESS;
}

static void Close(vlc_object_t *obj)
{
    (void)obj;

    if (g_stop)
        SetEvent(g_stop);

    if (g_thread) {
        WaitForSingleObject(g_thread, 2000);
        CloseHandle(g_thread);
        g_thread = NULL;
    }

    if (g_stop) {
        CloseHandle(g_stop);
        g_stop = NULL;
    }

    g_video = NULL;
    g_parent = NULL;
    g_top = NULL;
}
