#include <opencv2/opencv.hpp>
#include "Dashboard.h"
#include "DMSMonitor.h"
#include "DMSHUD.h"
#include <iostream>

int main() {
    std::cout << "[INFO] Loading DMS Models..." << std::endl;
    // Инициализация DMS
    DMSMonitor dms("models/deploy.prototxt", 
                   "models/res10_300x300_ssd_iter_140000.caffemodel", 
                   "models/haarcascade_eye.xml");
    DMSHUD dmsHud;

    // Инициализация Dashboard (Панели приборов)
    Dashboard dashboard;

    std::cout << "[INFO] Starting WebCamera..." << std::endl;
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "[ERROR] Could not open webcam!" << std::endl;
        return -1;
    }

    cv::Mat cameraFrame;
    
    // Главный цикл
    while (true) {
        cap >> cameraFrame;
        if (cameraFrame.empty()) {
            std::cerr << "[ERROR] Empty frame from webcam!" << std::endl;
            break;
        }

        // Обрезаем кадр по центру до пропорций 4:3, чтобы избежать сплющивания (если камера 16:9)
        cv::Mat croppedCamera;
        if (cameraFrame.cols * 3 > cameraFrame.rows * 4) { 
            int targetWidth = cameraFrame.rows * 4 / 3;
            int xOffset = (cameraFrame.cols - targetWidth) / 2;
            croppedCamera = cameraFrame(cv::Rect(xOffset, 0, targetWidth, cameraFrame.rows));
        } else {
            int targetHeight = cameraFrame.cols * 3 / 4;
            int yOffset = (cameraFrame.rows - targetHeight) / 2;
            croppedCamera = cameraFrame(cv::Rect(0, yOffset, cameraFrame.cols, targetHeight));
        }

        // Приводим к размеру 640x480 для идеального совпадения с Dashboard
        cv::Mat processedCamera;
        cv::resize(croppedCamera, processedCamera, cv::Size(640, 480));

        // Создаем общий canvas 960x480
        cv::Mat canvas = cv::Mat::zeros(480, 960, CV_8UC3);

        // 1. Левая половина: Dashboard (тестовые данные)
        cv::Mat leftHalf = canvas(cv::Rect(0, 0, 320, 480));
        
        // Передаем тестовые данные (нормальное вождение)
        double speed = 85.0;
        double rpm = 3000.0;
        double temp = 90.0;
        double fuel = 60.0;
        double throttle = 30.0;
        std::string style = "NORMAL";
        
        dashboard.draw(leftHalf, speed, rpm, temp, fuel, throttle, style);

        // 2. Правая половина: DMS
        // Анализируем уже обрезанный кадр (чтобы координаты лица точно совпадали)
        DriverState state = dms.analyze(processedCamera);
        dmsHud.draw(canvas, processedCamera, state);

        // Показываем результат
        cv::imshow("Real-Car ADAS Monitor", canvas);

        // Выход по клавише 'q'
        if (cv::waitKey(30) == 'q') {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}