// color.h
#pragma once
#include <cstdint>

struct RGB { int r, g, b; };
struct HSV { float h, s, v; };

HSV  RgbToHsv(RGB in);
bool IsTargetColor(const uint8_t* bgra, int tolH, int tolS, int tolV);