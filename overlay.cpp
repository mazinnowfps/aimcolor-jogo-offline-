#include "overlay.h"
#include <atomic>

static HWND   g_hwnd    = nullptr;
static HANDLE g_thread  = nullptr;
static DWORD  g_tid     = 0;
static std::atomic<int>  g_radius { 150 };
static std::atomic<bool> g_visible{ true  };
static std::atomic<bool> g_follow { false }; // false = centro fixo
static std::atomic<bool> g_alive  { false };
static std::atomic<bool> g_ready  { false };

static const COLORREF COLORKY = RGB(255, 0, 255);
static const int      WND_SIZE = 1400;

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
        case WM_ERASEBKGND: return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(h, &ps);
            RECT rc; GetClientRect(h, &rc);

            HBRUSH bg = CreateSolidBrush(COLORKY);
            FillRect(hdc, &rc, bg);
            DeleteObject(bg);

            if (g_visible.load()) {
                int r  = g_radius.load();
                int cx = rc.right  / 2;
                int cy = rc.bottom / 2;

                HPEN   pen  = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
                HPEN   oldP = (HPEN)SelectObject(hdc, pen);
                HBRUSH oldB = (HBRUSH)SelectObject(hdc,
                                   GetStockObject(NULL_BRUSH));
                Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
                SelectObject(hdc, oldP);
                SelectObject(hdc, oldB);
                DeleteObject(pen);

                HPEN pen2 = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                oldP = (HPEN)SelectObject(hdc, pen2);
                MoveToEx(hdc, cx - 4, cy, nullptr);
                LineTo  (hdc, cx + 4, cy);
                MoveToEx(hdc, cx, cy - 4, nullptr);
                LineTo  (hdc, cx, cy + 4);
                SelectObject(hdc, oldP);
                DeleteObject(pen2);
            }
            EndPaint(h, &ps);
            return 0;
        }
        case WM_DESTROY:
            g_alive = false;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

static void RepositionWindow() {
    if (!g_hwnd) return;

    if (g_follow.load()) {
        POINT p; GetCursorPos(&p);
        SetWindowPos(g_hwnd, HWND_TOPMOST,
                     p.x - WND_SIZE / 2, p.y - WND_SIZE / 2,
                     0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    } else {
        int sw = GetSystemMetrics(SM_CXSCREEN);
        int sh = GetSystemMetrics(SM_CYSCREEN);
        SetWindowPos(g_hwnd, HWND_TOPMOST,
                     sw / 2 - WND_SIZE / 2, sh / 2 - WND_SIZE / 2,
                     0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

static DWORD WINAPI OverlayThread(LPVOID) {
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = GetModuleHandleW(nullptr);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"AimColorOverlayWnd";
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT |
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        L"AimColorOverlayWnd", L"", WS_POPUP,
        0, 0, WND_SIZE, WND_SIZE,
        nullptr, nullptr, wc.hInstance, nullptr);

    if (!g_hwnd) return 1;

    SetLayeredWindowAttributes(g_hwnd, COLORKY, 0, LWA_COLORKEY);
    ShowWindow(g_hwnd, SW_SHOWNOACTIVATE);
    UpdateWindow(g_hwnd);

    g_alive = true;
    g_ready = true;

    MSG msg;
    while (g_alive.load()) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { g_alive = false; break; }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        RepositionWindow();
        InvalidateRect(g_hwnd, nullptr, FALSE);
        Sleep(16);
    }

    if (g_hwnd) { DestroyWindow(g_hwnd); g_hwnd = nullptr; }
    UnregisterClassW(L"AimColorOverlayWnd", GetModuleHandleW(nullptr));
    g_ready = false;
    return 0;
}

bool OverlayCreate(HINSTANCE) {
    if (g_thread) return true;
    g_thread = CreateThread(nullptr, 0, OverlayThread, nullptr,
                            CREATE_SUSPENDED, &g_tid);
    if (!g_thread) return false;
    ResumeThread(g_thread);
    for (int i = 0; i < 200 && !g_ready.load(); ++i) Sleep(10);
    return g_ready.load();
}

void OverlaySetRadius(int r)             { g_radius  = r; }
void OverlaySetVisible(bool v)           { g_visible = v; }
void OverlaySetFollowCursor(bool follow) { g_follow  = follow; }

void OverlayDestroy() {
    g_alive = false;
    if (g_tid) PostThreadMessageW(g_tid, WM_QUIT, 0, 0);
    if (g_thread) {
        WaitForSingleObject(g_thread, 1000);
        CloseHandle(g_thread);
        g_thread = nullptr;
    }
}