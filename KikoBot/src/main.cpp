#include <iostream>
#include <windows.h>
#include <opencv2/opencv.hpp>
#include "utils.h"

// Forward declaration instead of including color_detector.h
std::vector<cv::Rect> detectPorings(cv::Mat& frame);

// Move cursor + click using SendInput
void clickAt(HWND hwnd, int x, int y) {
    // Bring game window to front
    SetForegroundWindow(hwnd);

    // Move cursor
    SetCursorPos(x, y);

    // Real click
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, inputs, sizeof(INPUT));
}

int main() {
    std::cout << "[INFO] Starting KikoBot...\n";

    // Attach to Ragnarok client
    HWND hwndRO = FindWindowW(NULL, L"Ragnarok");
    if (!hwndRO) {
        std::cerr << "[ERROR] Could not find Ragnarok window.\n";
        return -1;
    }
    std::cout << "[INFO] Attached to Ragnarok window.\n";

    bool paused = false;

    while (true) {
        // Hotkey checks
        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            paused = !paused;
            std::cout << (paused ? "[INFO] Paused.\n" : "[INFO] Resumed.\n");
            Sleep(300); // debounce
        }
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            std::cout << "[INFO] Stopping bot.\n";
            break;
        }

        if (paused) {
            Sleep(200);
            continue;
        }

        cv::Mat frame = captureFrame(hwndRO);
        if (frame.empty()) {
            std::cerr << "[ERROR] Capture failed.\n";
            Sleep(500);
            continue;
        }

        // Detect porings
        auto boxes = detectPorings(frame);

        if (!boxes.empty()) {
            for (const auto& box : boxes) {
                cv::Point center(box.x + box.width/2, box.y + box.height/2);
                POINT pt{center.x, center.y};
                ClientToScreen(hwndRO, &pt);
                clickAt(hwndRO, pt.x, pt.y);
                std::cout << "[ATTACK] Clicked at " << pt.x << "," << pt.y << "\n";
                Sleep(300); // cooldown between clicks
            }
        } else {
            // Exploration: random clicks inside client area
            RECT rect;
            GetClientRect(hwndRO, &rect);
            int cx = rand() % rect.right;
            int cy = rand() % rect.bottom;
            POINT pt{cx, cy};
            ClientToScreen(hwndRO, &pt);
            clickAt(hwndRO, pt.x, pt.y);
            std::cout << "[EXPLORE] Clicked at " << pt.x << "," << pt.y << "\n";
            Sleep(500);
        }

        // Show preview
        cv::imshow("KikoBot Preview", frame);
        cv::resizeWindow("KikoBot Preview", 700, 500);
        if (cv::waitKey(30) == 27) break; // ESC to quit
    }

    return 0;
}