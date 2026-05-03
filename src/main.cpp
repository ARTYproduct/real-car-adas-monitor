#include <opencv2/opencv.hpp>
#include "Dashboard.h"
#include <iostream>

int main() {
    std::cout << "[INFO] Testing Dashboard..." << std::endl;

    // Создаем пустой черный кадр 640x480
    cv::Mat frame = cv::Mat::zeros(480, 640, CV_8UC3);

    // Создаем экземпляр панели приборов
    Dashboard dashboard;

    // Тестовые данные, чтобы проверить красные зоны и предупреждения
    double testSpeed = 105.0; // > 90 (Красная дуга)
    double testRpm = 4800.0;  // > 4500 (Красная зона)
    double testTemp = 105.0;  // > 100 (Красный бар + Warning)
    double testFuel = 10.0;   // < 15 (Предупреждение)
    double testThrottle = 85.0;
    std::string testStyle = "AGGRESSIVE"; // Цвет красный

    // Отрисовка
    dashboard.draw(frame, testSpeed, testRpm, testTemp, testFuel, testThrottle, testStyle);

    // Показ
    cv::imshow("Dashboard Test", frame);

    std::cout << "[INFO] Press any key on the window to exit." << std::endl;
    cv::waitKey(0);

    return 0;
}