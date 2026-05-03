#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class Dashboard {
public:
    Dashboard();
    
    // Главный метод для отрисовки всех элементов на кадре
    void draw(cv::Mat& frame, double speed, double rpm, double temp, double fuel, double throttle, const std::string& drivingStyle);

private:
    // Отрисовка одного кругового прибора (спидометр, тахометр)
    void drawGauge(cv::Mat& frame, cv::Point center, int radius, double value, double maxVal, const std::string& label, const cv::Scalar& color);
    
    // Отрисовка одной горизонтальной полосы (температура, топливо, заслонка)
    void drawLinearGauge(cv::Mat& frame, cv::Rect rect, double value, double maxVal, const std::string& label);
};
