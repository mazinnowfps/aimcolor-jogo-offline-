#include <windows.h>
#include <cstdio>
#include <conio.h>

#include "shared.h"

static Config* g_cfg = nullptr;

static void PrintMenu() {
    system("cls");
    std::printf("============ AimColorLab ============\n");
    std::printf("[F1]  Ligar/Desligar ........ %s\n",
                g_cfg->enabled ? "LIGADO" : "DESLIGADO");
    std::printf("[F2]  Nudge mouse +20px\n");
    std::printf("[F3]  Tolerancia H .......... %d\n", g_cfg->tolH);
    std::printf("[F4]  Tolerancia H + (max 180)\n");
    std::printf("[F5]  Tolerancia H -\n");
    std::printf("[F6]  FOV .................... %d px\n", g_cfg->fov);
    std::printf("[F7]  FOV + (max 400)\n");
    std::printf("[F8]  FOV - (min 30)\n");
    std::printf("[F9]  Mostrar circulo FOV .... %s\n",
                g_cfg->showFov ? "ON" : "OFF");
    std::printf("[F10] FOV segue mouse? ....... %s\n",
                g_cfg->fovFollowMouse ? "SIM (follow)" : "NAO (centro fixo)");
    std::printf("-------------------------------------\n");
    std::printf("FORCA (sensibilidade) ........ %.2f\n",
                g_cfg->sensitivity);
    std::printf("  [A] Forca +   [S] Forca -\n");
    std::printf("SMOOTHING (passos) ........... %d\n",
                g_cfg->smooth);
    std::printf("  [Z] Smooth +  [X] Smooth -\n");
    std::printf("VELOCIDADE (delay passo ms) .. %d\n",
                g_cfg->stepDelayMs);
    std::printf("  [D] Delay +   [C] Delay -\n");
    std::printf("=====================================\n");
    std::printf("[Q] Sair\n");
}

int main() {
    HANDLE map = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr,
                                    PAGE_READWRITE, 0, sizeof(Config),
                                    SHARED_NAME);
    if (!map) { std::printf("Erro map\n"); return 1; }

    g_cfg = (Config*)MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0,
                                   sizeof(Config));
    if (!g_cfg) { std::printf("Erro view\n"); return 1; }

    if (g_cfg->fov == 0) {
        g_cfg->enabled        = 0;
        g_cfg->tolH           = 10;
        g_cfg->tolS           = 15;
        g_cfg->tolV           = 15;
        g_cfg->fov            = 100;
        g_cfg->smooth         = 6;
        g_cfg->sensitivity    = 0.25f;
        g_cfg->deadzone       = 1;
        g_cfg->showFov        = 1;
        g_cfg->nudgeRequest   = 0;
        g_cfg->stepDelayMs    = 1;
        g_cfg->fovFollowMouse = 0;
    }

    HMODULE h = LoadLibraryA("RzCommon.dll");
    if (!h) {
        std::printf("Falha DLL: %lu\n", GetLastError());
        return 1;
    }

    bool quit = false;
    while (!quit) {
        PrintMenu();
        int k = _getch();
        if (k == 0 || k == 224) {
            int k2 = _getch();
            switch (k2) {
                case 59: g_cfg->enabled ^= 1; break;                 // F1
                case 60: g_cfg->nudgeRequest = 1; break;             // F2
                case 61: if (g_cfg->tolH > 1) g_cfg->tolH--; break;  // F3
                case 62: if (g_cfg->tolH < 180) g_cfg->tolH++; break;// F4
                case 63: if (g_cfg->tolH > 1) g_cfg->tolH--; break;  // F5
                case 64: break;                                      // F6
                case 65: if (g_cfg->fov < 400) g_cfg->fov += 10; break;// F7
                case 66: if (g_cfg->fov > 30)  g_cfg->fov -= 10; break;// F8
                case 67: g_cfg->showFov ^= 1; break;                 // F9
                case 68: g_cfg->fovFollowMouse ^= 1; break;          // F10
            }
        } else {
            switch (k) {
                case '1': g_cfg->enabled ^= 1; break;
                case '2': g_cfg->nudgeRequest = 1; break;
                case '3': if (g_cfg->tolH > 1) g_cfg->tolH--; break;
                case '4': if (g_cfg->tolH < 180) g_cfg->tolH++; break;
                case '5': if (g_cfg->fov > 30) g_cfg->fov -= 10; break;
                case '6': if (g_cfg->fov < 400) g_cfg->fov += 10; break;
                case '7': g_cfg->showFov ^= 1; break;

                // TOGGLE FOV-SEGUE-MOUSE
                case '8': case 'm': case 'M':
                    g_cfg->fovFollowMouse ^= 1;
                    break;

                // FORÇA
                case 'a': case 'A':
                    g_cfg->sensitivity += 0.05f;
                    if (g_cfg->sensitivity > 1.0f) g_cfg->sensitivity = 1.0f;
                    break;
                case 's': case 'S':
                    g_cfg->sensitivity -= 0.05f;
                    if (g_cfg->sensitivity < 0.05f) g_cfg->sensitivity = 0.05f;
                    break;

                // SMOOTHING
                case 'z': case 'Z':
                    if (g_cfg->smooth < 30) g_cfg->smooth++;
                    break;
                case 'x': case 'X':
                    if (g_cfg->smooth > 1) g_cfg->smooth--;
                    break;

                // VELOCIDADE
                case 'd': case 'D':
                    if (g_cfg->stepDelayMs < 20) g_cfg->stepDelayMs++;
                    break;
                case 'c': case 'C':
                    if (g_cfg->stepDelayMs > 0) g_cfg->stepDelayMs--;
                    break;

                case 'q': case 'Q': quit = true; break;
            }
        }
    }

    FreeLibrary(h);
    UnmapViewOfFile(g_cfg);
    CloseHandle(map);
    return 0;
}