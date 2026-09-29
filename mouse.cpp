// mouse.cpp
#include "mouse.h"
#include <windows.h>
#include <thread>
#include <chrono>
#include <cmath>

void MoveMouseRelative(int dx, int dy) {
    INPUT in{};
    in.type       = INPUT_MOUSE;
    in.mi.dwFlags = MOUSEEVENTF_MOVE;
    in.mi.dx      = dx;
    in.mi.dy      = dy;
    SendInput(1, &in, sizeof(INPUT));
}

// interpola com ease-out quadrático (suave, sem tremor)
void MoveMouseSmooth(int dx, int dy, int steps, int stepDelayMs) {
    if (steps < 1) steps = 1;
    if (stepDelayMs < 1) stepDelayMs = 1;

    float accX = 0.f, accY = 0.f;
    int   sentX = 0,  sentY = 0;

    for (int i = 1; i <= steps; ++i) {
        // ease-out: começa rápido, desacelera
        float t  = (float)i / (float)steps;
        float et = 1.0f - (1.0f - t) * (1.0f - t); // 1-(1-t)^2

        int targetX = (int)std::lround(dx * et);
        int targetY = (int)std::lround(dy * et);

        int stepX = targetX - sentX;
        int stepY = targetY - sentY;

        if (stepX != 0 || stepY != 0) {
            MoveMouseRelative(stepX, stepY);
            sentX = targetX;
            sentY = targetY;
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(stepDelayMs));
    }

    // garante chegada exata ao alvo
    int fx = dx - sentX;
    int fy = dy - sentY;
    if (fx != 0 || fy != 0) MoveMouseRelative(fx, fy);
}