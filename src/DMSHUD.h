#pragma once
#include <opencv2/opencv.hpp>
#include "DMSMonitor.h"

class DMSHUD {
public:
    DMSHUD();
    
    // Отрисовка интерфейса DMS. 
    // canvas - общий кадр 1280x480.
    // cameraFrame - 640x480 кадр с камеры (размещается справа).
    // state - текущее состояние водителя.
    void draw(cv::Mat& canvas, const cv::Mat& cameraFrame, const DriverState& state);
};
