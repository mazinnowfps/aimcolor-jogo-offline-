// capture.h
#pragma once
#include <windows.h>
#include <cstdint>
#include <vector>

class ScreenCapture {
public:
    ScreenCapture(int x, int y, int w, int h);
    ~ScreenCapture();
    const std::vector<uint8_t>& Grab();
    int Width()  const { return w_; }
    int Height() const { return h_; }
private:
    int x_, y_, w_, h_;
    HDC     hdcScreen_ = nullptr;
    HDC     hdcMem_    = nullptr;
    HBITMAP hBitmap_   = nullptr;
    BITMAPINFO bmi_{};
    std::vector<uint8_t> buffer_;
};