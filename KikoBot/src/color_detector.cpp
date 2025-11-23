#ifndef CONFIG_DIR
#define CONFIG_DIR "D:/KikoBot/config"
#endif

#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdexcept>

using json = nlohmann::json;

struct ColorConfig {
    cv::Scalar hsvLow;
    cv::Scalar hsvHigh;
    int minArea;
    int maxArea;
};

static cv::Scalar readHSV3(const json& arr, const char* name) {
    if (!arr.is_array() || arr.size() < 3) {
        throw std::runtime_error(std::string("Invalid HSV array for ") + name);
    }
    return cv::Scalar(arr[0].get<double>(), arr[1].get<double>(), arr[2].get<double>());
}

static int getIntOrDefault(const json& j, const char* key, int def) {
    if (j.contains(key) && j[key].is_number_integer()) return j[key].get<int>();
    return def;
}

static ColorConfig loadColorConfig(const std::string& monsterName) {
    std::ifstream in(std::string(CONFIG_DIR) + "/colors.json");
    if (!in.is_open()) throw std::runtime_error("Failed to open colors.json");

    json j; in >> j;
    if (!j.contains(monsterName)) throw std::runtime_error("Monster config not found: " + monsterName);

    const json& cfg = j[monsterName];
    cv::Scalar hsvLow  = readHSV3(cfg["lower_hsv"], "lower_hsv");
    cv::Scalar hsvHigh = readHSV3(cfg["upper_hsv"], "upper_hsv");

    return ColorConfig{
        hsvLow, hsvHigh,
        getIntOrDefault(cfg, "min_area", 250),
        getIntOrDefault(cfg, "max_area", 1000000)
    };
}

std::vector<cv::Rect> detectPorings(cv::Mat& frame) {
    static ColorConfig cfg = loadColorConfig("poring");

    cv::Mat hsv; cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask; cv::inRange(hsv, cfg.hsvLow, cfg.hsvHigh, mask);

    // Optional cleanup
    cv::erode(mask, mask, cv::Mat(), cv::Point(-1,-1), 1);
    cv::dilate(mask, mask, cv::Mat(), cv::Point(-1,-1), 1);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Rect> boxes;
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area < cfg.minArea || area > cfg.maxArea) continue;

        cv::Rect box = cv::boundingRect(contour);
        boxes.push_back(box);
        cv::rectangle(frame, box, cv::Scalar(0,255,0), 2);
    }

    return boxes;
}