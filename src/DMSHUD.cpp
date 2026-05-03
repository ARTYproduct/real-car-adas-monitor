#include "DMSHUD.h"
#include <string>

DMSHUD::DMSHUD() {}

void DMSHUD::draw(cv::Mat& canvas, const cv::Mat& cameraFrame, const DriverState& state) {
    // Вставляем кадр с камеры в правую часть (начиная с x=320)
    cv::Rect rightHalf(320, 0, 640, 480);
    cv::Mat dmsCanvas = canvas(rightHalf);

    // Если размер не совпадает, ресайзим (на случай, если камера не 640x480)
    cv::Mat resizedCamera;
    if (cameraFrame.cols != 640 || cameraFrame.rows != 480) {
        cv::resize(cameraFrame, resizedCamera, cv::Size(640, 480));
    } else {
        resizedCamera = cameraFrame;
    }

    // Копируем кадр
    resizedCamera.copyTo(dmsCanvas);

    // Отрисовка маркеров лица (углы)
    if (state.face_detected) {
        cv::Rect f = state.face_rect;
        cv::Scalar faceColor = cv::Scalar(0, 255, 0); // Зеленый в норме

        // Линии углов (длина 20 пикселей)
        int len = 20;
        int t = 2; // толщина

        // Верхний левый
        cv::line(dmsCanvas, cv::Point(f.x, f.y), cv::Point(f.x + len, f.y), faceColor, t, cv::LINE_AA);
        cv::line(dmsCanvas, cv::Point(f.x, f.y), cv::Point(f.x, f.y + len), faceColor, t, cv::LINE_AA);

        // Верхний правый
        cv::line(dmsCanvas, cv::Point(f.x + f.width, f.y), cv::Point(f.x + f.width - len, f.y), faceColor, t, cv::LINE_AA);
        cv::line(dmsCanvas, cv::Point(f.x + f.width, f.y), cv::Point(f.x + f.width, f.y + len), faceColor, t, cv::LINE_AA);

        // Нижний левый
        cv::line(dmsCanvas, cv::Point(f.x, f.y + f.height), cv::Point(f.x + len, f.y + f.height), faceColor, t, cv::LINE_AA);
        cv::line(dmsCanvas, cv::Point(f.x, f.y + f.height), cv::Point(f.x, f.y + f.height - len), faceColor, t, cv::LINE_AA);

        // Нижний правый
        cv::line(dmsCanvas, cv::Point(f.x + f.width, f.y + f.height), cv::Point(f.x + f.width - len, f.y + f.height), faceColor, t, cv::LINE_AA);
        cv::line(dmsCanvas, cv::Point(f.x + f.width, f.y + f.height), cv::Point(f.x + f.width, f.y + f.height - len), faceColor, t, cv::LINE_AA);
    }

    // Индикаторы состояния (вверху справа или слева внутри DMSCanvas)
    int font = cv::FONT_HERSHEY_DUPLEX;
    cv::putText(dmsCanvas, "DMS STATUS", cv::Point(20, 30), font, 0.6, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    
    cv::Scalar eyeColor = state.eyes_open ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
    cv::putText(dmsCanvas, state.eyes_open ? "EYES: OPEN" : "EYES: CLOSED", cv::Point(20, 60), font, 0.5, eyeColor, 1, cv::LINE_AA);

    cv::Scalar headColor = state.looking_forward ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
    cv::putText(dmsCanvas, state.looking_forward ? "HEAD: FORWARD" : "HEAD: TURNED", cv::Point(20, 85), font, 0.5, headColor, 1, cv::LINE_AA);

    // Алерт: Отвлечение (Красная полоса снизу)
    if (state.alert_distracted) {
        cv::Mat overlay = dmsCanvas.clone();
        cv::rectangle(overlay, cv::Rect(0, 430, 640, 50), cv::Scalar(0, 0, 255), -1);
        cv::addWeighted(overlay, 0.6, dmsCanvas, 0.4, 0, dmsCanvas);
        
        cv::putText(dmsCanvas, "DISTRACTION ALERT", cv::Point(220, 465), font, 0.7, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    }

    // Алерт: Усталость (Оранжевая рамка и плашка в центре)
    if (state.alert_drowsy) {
        cv::rectangle(dmsCanvas, cv::Rect(0, 0, 640, 480), cv::Scalar(0, 165, 255), 8); // Оранжевая рамка

        cv::Rect alertBox(170, 200, 300, 60);
        cv::rectangle(dmsCanvas, alertBox, cv::Scalar(0, 165, 255), -1); // Заливка
        cv::rectangle(dmsCanvas, alertBox, cv::Scalar(255, 255, 255), 2); // Граница
        cv::putText(dmsCanvas, "DROWSINESS ALERT", cv::Point(190, 238), font, 0.8, cv::Scalar(0, 0, 0), 2, cv::LINE_AA);
    }
}
