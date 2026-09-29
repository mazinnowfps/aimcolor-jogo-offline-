#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct Config {
    volatile int  enabled;
    volatile int  tolH;
    volatile int  tolS;
    volatile int  tolV;
    volatile int  fov;
    volatile int  smooth;
    volatile float sensitivity;
    volatile int  deadzone;
    volatile int  showFov;
    volatile int  nudgeRequest;
    volatile int  stepDelayMs;
    volatile int  fovFollowMouse;   // <-- NOVO: 0 = centro fixo, 1 = segue mouse
    volatile int  pad[5];
};
#pragma pack(pop)

#define SHARED_NAME  L"Global\\AimColorLab_Shared"