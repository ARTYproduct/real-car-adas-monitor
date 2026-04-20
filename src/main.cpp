#include <opencv2/opencv.hpp>
#include "OBDParser.hpp"
#include <iostream>
#include <chrono>

void drawDashboard(cv::Mat& frame, const VehicleData& data) {
    // 1. Создаем полупрозрачную подложку (Overlay)
    cv::Mat overlay = frame.clone();
    cv::rectangle(overlay, cv::Point(10, 10), cv::Point(300, 180), cv::Scalar(0, 0, 0), -1);
    
    double alpha = 0.6; // Прозрачность
    cv::addWeighted(overlay, alpha, frame, 1.0 - alpha, 0, frame);

    // 2. Выводим текст с красивым шрифтом
    int font = cv::FONT_HERSHEY_DUPLEX;
    double scale = 0.7;
    cv::Scalar color = cv::Scalar(255, 255, 255); 
    int thickness = 1;

    cv::putText(frame, "SYSTEM TELEMETRY", cv::Point(20, 40), font, 0.8, cv::Scalar(255, 255, 255), 2);
    cv::putText(frame, "--------------", cv::Point(20, 55), font, scale, cv::Scalar(255, 255, 255), 1);
    
    cv::putText(frame, "Speed: " + std::to_string(data.speed) + " km/h", cv::Point(30, 85), font, scale, color, thickness);
    cv::putText(frame, "RPM:   " + std::to_string(data.rpm), cv::Point(30, 115), font, scale, color, thickness);
    cv::putText(frame, "Load:  " + std::to_string((int)data.throttle) + " %", cv::Point(30, 145), font, scale, color, thickness);
    cv::putText(frame, "Temp:  " + std::to_string((int)data.temp) + " C", cv::Point(30, 175), font, scale, color, thickness);
}

int main() {
    OBDParser parser("data/obd_data.csv");
    std::vector<VehicleData> records = parser.parse();
    if (records.empty()) return -1;

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) return -1;

    cv::Mat frame;
    int dataIndex = 0;
    
    // Для синхронизации времени
    auto startTime = std::chrono::steady_clock::now();
    double firstTimestamp = records[0].timestamp;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // Вычисляем, сколько мс прошло с начала запуска программы
        auto currentTime = std::chrono::steady_clock::now();
        double elapsedMs = std::chrono::duration<double, std::milli>(currentTime - startTime).count();

        // Ищем в датасете строку, которая соответствует текущему моменту времени
        // (Синхронизация по полю Timestampms)
        while (dataIndex < records.size() - 1 && 
               (records[dataIndex].timestamp - firstTimestamp) < elapsedMs) {
            dataIndex++;
        }

        // Отрисовка Dashboard
        drawDashboard(frame, records[dataIndex]);

        cv::imshow("Real-Time ADAS Monitor", frame);

        if (cv::waitKey(1) == 'q') break; // Минимальная задержка для плавности
    }

    return 0;
}