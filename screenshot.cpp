#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <windows.h>
#include "screenshot.h"

Screenshot::Screenshot() {
    SetProcessDPIAware();

    const HDC hScreen = GetDC(nullptr);
    if (hScreen == nullptr) {
        throw std::runtime_error("GetDC failed");
    }

    width = GetDeviceCaps(hScreen, HORZRES);
    height = GetDeviceCaps(hScreen, VERTRES);


    const HDC hDC = CreateCompatibleDC(hScreen);
    if (hDC == nullptr) {
        ReleaseDC(nullptr, hScreen);
        throw std::runtime_error("CreateCompatibleDC failed");
    }

    const HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
    if (!hBitmap) {
        DeleteDC(hDC);
        ReleaseDC(nullptr, hScreen);
        throw std::runtime_error("CreateCompatibleBitmap failed");
    }

    const HGDIOBJ oldObj = SelectObject(hDC, hBitmap);

    if (!BitBlt(hDC, 0, 0, width, height, hScreen, 0, 0, SRCCOPY)) {
        SelectObject(hDC, oldObj);
        DeleteObject(hBitmap);
        DeleteDC(hDC);
        ReleaseDC(nullptr, hScreen);
        throw std::runtime_error("BitBlt failed");
    }

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    pixels.resize(width * height * 4);

    if (!GetDIBits(hDC, hBitmap, 0, height, pixels.data(), &bmi, DIB_RGB_COLORS)) {
        SelectObject(hDC, oldObj);
        DeleteObject(hBitmap);
        DeleteDC(hDC);
        ReleaseDC(nullptr, hScreen);
        throw std::runtime_error("GetDIBits failed");
    }

    SelectObject(hDC, oldObj);
    DeleteObject(hBitmap);
    DeleteDC(hDC);
    ReleaseDC(nullptr, hScreen);
}

std::array<unsigned char, 3> Screenshot::get_rgb(const int x, const int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        throw std::out_of_range("pixel coordinates out of range");
    }

    const int offset = (y * width + x) * 4;
    return {pixels[offset + 2], pixels[offset + 1], pixels[offset + 0]};
}