#include <iostream>
#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>

int main() {
    std::cout << "=== ADAS System Starter ===" << std::endl;
    std::cout << "OpenCV version: " << CV_VERSION << std::endl;
    
    // Простая проверка среды ONNX
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "TestSession");
    std::cout << "ONNX Runtime is ready!" << std::endl;
    std::cout << "Running on Mac M1 (Apple Silicon)" << std::endl;

    return 0;
}