// capture.cpp
#include "capture.h"

ScreenCapture::ScreenCapture(int x, int y, int w, int h)
    : x_(x), y_(y), w_(w), h_(h) {
    hdcScreen_ = GetDC(nullptr);
    hdcMem_    = CreateCompatibleDC(hdcScreen_);
    hBitmap_   = CreateCompatibleBitmap(hdcScreen_, w_, h_);
    SelectObject(hdcMem_, hBitmap_);

    bmi_.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi_.bmiHeader.biWidth       = w_;
    bmi_.bmiHeader.biHeight      = -h_;
    bmi_.bmiHeader.biPlanes      = 1;
    bmi_.bmiHeader.biBitCount    = 32;
    bmi_.bmiHeader.biCompression = BI_RGB;

    buffer_.resize(static_cast<size_t>(w_) * h_ * 4);
}

ScreenCapture::~ScreenCapture() {
    if (hBitmap_)   DeleteObject(hBitmap_);
    if (hdcMem_)    DeleteDC(hdcMem_);
    if (hdcScreen_) ReleaseDC(nullptr, hdcScreen_);
}

const std::vector<uint8_t>& ScreenCapture::Grab() {
    BitBlt(hdcMem_, 0, 0, w_, h_, hdcScreen_, x_, y_, SRCCOPY);
    GetDIBits(hdcMem_, hBitmap_, 0, h_, buffer_.data(), &bmi_, DIB_RGB_COLORS);
    return buffer_;
}