#include <windows.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <climits>
#include <cmath>

#include "capture.h"
#include "color.h"
#include "mouse.h"
#include "overlay.h"
#include "shared.h"

static std::atomic<bool> g_running{false};
static std::thread       g_worker;
static HANDLE            g_map  = nullptr;
static Config*           g_cfg  = nullptr;

static constexpr int MAX_FOV = 400;

static void Worker(HMODULE hSelf) {
    OverlayCreate(hSelf);

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    // captura fixa centralizada (usada no modo centro-fixo)
    int capFixedX = sw / 2 - MAX_FOV;
    int capFixedY = sh / 2 - MAX_FOV;
    ScreenCapture capFixed(capFixedX, capFixedY, MAX_FOV * 2, MAX_FOV * 2);

    int  lastFov    = -1;
    bool lastVis    = false;
    bool lastFollow = false;

    while (g_running.load(std::memory_order_relaxed)) {
        int fov       = g_cfg->fov;
        int tolH      = g_cfg->tolH;
        int tolS      = g_cfg->tolS;
        int tolV      = g_cfg->tolV;
        int smooth    = g_cfg->smooth;
        int deadzone  = g_cfg->deadzone;
        int stepMs    = g_cfg->stepDelayMs;
        float sens    = g_cfg->sensitivity;
        bool enabled  = g_cfg->enabled != 0;
        bool follow   = g_cfg->fovFollowMouse != 0;

        if (fov != lastFov) { OverlaySetRadius(fov); lastFov = fov; }
        bool vis = g_cfg->showFov != 0;
        if (vis != lastVis) { OverlaySetVisible(vis); lastVis = vis; }
        if (follow != lastFollow) {
            OverlaySetFollowCursor(follow);
            lastFollow = follow;
        }

        if (g_cfg->nudgeRequest) {
            MoveMouseRelative(20, 0);
            g_cfg->nudgeRequest = 0;
        }

        if (enabled) {
            int cx = MAX_FOV;
            int cy = MAX_FOV;
            const std::vector<uint8_t>* bufPtr = nullptr;

            ScreenCapture* capDynamic = nullptr;

            if (follow) {
                // captura em volta do cursor
                POINT p; GetCursorPos(&p);
                int capX = p.x - MAX_FOV;
                int capY = p.y - MAX_FOV;
                capDynamic = new ScreenCapture(capX, capY,
                                               MAX_FOV * 2, MAX_FOV * 2);
                bufPtr = &capDynamic->Grab();
            } else {
                // captura fixa no centro da tela
                bufPtr = &capFixed.Grab();
            }

            const auto& buf = *bufPtr;

            int bestX = -1, bestY = -1;
            int bestD2 = INT_MAX;
            int fov2 = fov * fov;

            for (int y = cy - fov; y <= cy + fov; y += 1) {
                if (y < 0 || y >= MAX_FOV * 2) continue;
                for (int x = cx - fov; x <= cx + fov; x += 1) {
                    if (x < 0 || x >= MAX_FOV * 2) continue;

                    int dx0 = x - cx;
                    int dy0 = y - cy;
                    int d2  = dx0 * dx0 + dy0 * dy0;
                    if (d2 > fov2) continue;

                    const uint8_t* px = buf.data() +
                        (static_cast<size_t>(y) * (MAX_FOV * 2) + x) * 4;

                    if (IsTargetColor(px, tolH, tolS, tolV)) {
                        if (d2 < bestD2) { bestD2 = d2; bestX = x; bestY = y; }
                    }
                }
            }

            if (bestX >= 0) {
                int dx = bestX - cx;
                int dy = bestY - cy;

                if (std::abs(dx) > deadzone || std::abs(dy) > deadzone) {
                    int mvx = (int)std::lround(dx * sens);
                    int mvy = (int)std::lround(dy * sens);

                    if (mvx == 0 && dx != 0) mvx = (dx > 0 ? 1 : -1);
                    if (mvy == 0 && dy != 0) mvy = (dy > 0 ? 1 : -1);

                    if (std::abs(mvx) > std::abs(dx)) mvx = dx;
                    if (std::abs(mvy) > std::abs(dy)) mvy = dy;

                    MoveMouseSmooth(mvx, mvy, smooth, stepMs);
                }
            }

            if (capDynamic) { delete capDynamic; capDynamic = nullptr; }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    OverlayDestroy();
}

BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hMod);

        g_map = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr,
                                   PAGE_READWRITE, 0, sizeof(Config),
                                   SHARED_NAME);
        if (!g_map) return TRUE;
        g_cfg = (Config*)MapViewOfFile(g_map, FILE_MAP_ALL_ACCESS, 0, 0,
                                       sizeof(Config));
        if (!g_cfg) return TRUE;

        if (g_cfg->fov == 0) {
            g_cfg->enabled        = 1;
            g_cfg->tolH           = 10;
            g_cfg->tolS           = 15;
            g_cfg->tolV           = 15;
            g_cfg->fov            = 100;
            g_cfg->smooth         = 6;
            g_cfg->sensitivity    = 0.25f;
            g_cfg->deadzone       = 1;
            g_cfg->showFov        = 1;
            g_cfg->stepDelayMs    = 1;
            g_cfg->fovFollowMouse = 0;   // padrão: centro fixo
        }

        g_running.store(true);
        g_worker = std::thread(Worker, hMod);
        g_worker.detach();
    } else if (reason == DLL_PROCESS_DETACH) {
        g_running.store(false);
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        if (g_cfg) UnmapViewOfFile(g_cfg);
        if (g_map) CloseHandle(g_map);
    }
    return TRUE;
}