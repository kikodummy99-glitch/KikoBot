#include "utils.h"
#include <windows.h>
#include <opencv2/opencv.hpp>

// Reliable client-area capture using GetDIBits to fill the buffer
cv::Mat captureFrame(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    HDC hdcWindow = GetDC(hwnd);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);

    BITMAPINFOHEADER bi{};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; // top-left origin
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    cv::Mat bgra(height, width, CV_8UC4);
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, width, height);
    SelectObject(hdcMemDC, hbmScreen);

    BitBlt(hdcMemDC, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);
    GetDIBits(hdcWindow, hbmScreen, 0, height, bgra.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);

    cv::Mat bgr;
    cv::cvtColor(bgra, bgr, cv::COLOR_BGRA2BGR);
    return bgr;
}