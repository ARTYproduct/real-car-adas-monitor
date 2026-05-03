#include "Dashboard.h"
#include <cmath>
#include <algorithm>

Dashboard::Dashboard() {}

void Dashboard::draw(cv::Mat& frame, double speed, double rpm, double temp, double fuel, double throttle, const std::string& drivingStyle) {
    // Полупрозрачная подложка на левую половину кадра
    cv::Mat overlay = frame.clone();
    cv::rectangle(overlay, cv::Rect(0, 0, 320, 480), cv::Scalar(20, 20, 20), -1);
    double alpha = 0.7;
    cv::addWeighted(overlay, alpha, frame, 1.0 - alpha, 0, frame);

    // Спидометр
    // Скорость 0-140. Цвет дуги: зеленый (<90), красный (>90)
    cv::Scalar speedColor = (speed > 90) ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0);
    drawGauge(frame, cv::Point(90, 100), 60, speed, 140.0, "Speed", speedColor);

    // Тахометр
    // RPM 0-6000. Красная зона после 4500
    cv::Scalar rpmColor = (rpm > 4500) ? cv::Scalar(0, 0, 255) : cv::Scalar(255, 255, 255);
    drawGauge(frame, cv::Point(230, 100), 60, rpm, 6000.0, "RPM", rpmColor);

    // Линейные приборы
    // Температура (0-120)
    drawLinearGauge(frame, cv::Rect(30, 220, 260, 20), temp, 120.0, "Temp (C)");
    
    // Топливо (0-100)
    drawLinearGauge(frame, cv::Rect(30, 270, 260, 20), fuel, 100.0, "Fuel (%)");
    
    // Дроссельная заслонка (0-100)
    drawLinearGauge(frame, cv::Rect(30, 320, 260, 20), throttle, 100.0, "Throttle (%)");

    // Стиль вождения
    cv::Scalar styleColor;
    if (drivingStyle == "NORMAL") styleColor = cv::Scalar(0, 255, 0); // Green
    else if (drivingStyle == "AGGRESSIVE") styleColor = cv::Scalar(0, 0, 255); // Red
    else if (drivingStyle == "SLOW") styleColor = cv::Scalar(255, 255, 0); // Cyan
    else styleColor = cv::Scalar(255, 255, 255); // White (fallback)

    cv::putText(frame, "Style: " + drivingStyle, cv::Point(30, 380), cv::FONT_HERSHEY_DUPLEX, 0.6, styleColor, 1, cv::LINE_AA);

    // Предупреждения
    int yOffset = 420;
    if (temp > 100.0) {
        cv::putText(frame, "WARNING: HIGH TEMP!", cv::Point(30, yOffset), cv::FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
        yOffset += 30;
    }
    if (fuel < 15.0) {
        cv::putText(frame, "WARNING: LOW FUEL!", cv::Point(30, yOffset), cv::FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    }
}

void Dashboard::drawGauge(cv::Mat& frame, cv::Point center, int radius, double value, double maxVal, const std::string& label, const cv::Scalar& color) {
    // Отрисовка фона круга
    cv::circle(frame, center, radius, cv::Scalar(50, 50, 50), 2, cv::LINE_AA);
    
    // Вычисление угла дуги и стрелки (от 135 до 45 градусов, итого 270 градусов дуги)
    // 0 внизу слева, max внизу справа
    double startAngle = 135.0; // В координатах OpenCV 0 это 3 часа, идем по часовой.
    double endAngle = 405.0;
    double range = endAngle - startAngle; // 270 градусов
    
    double valClamped = std::max(0.0, std::min(value, maxVal));
    double fraction = valClamped / maxVal;
    double currentAngle = startAngle + fraction * range;

    // Отрисовка активной дуги
    // ellipse(img, center, axes, angle, startAngle, endAngle, color, thickness, lineType)
    cv::ellipse(frame, center, cv::Size(radius, radius), 0, startAngle, currentAngle, color, 3, cv::LINE_AA);

    // Отрисовка стрелки
    double rad = currentAngle * CV_PI / 180.0;
    int x = center.x + (int)((radius - 10) * cos(rad));
    int y = center.y + (int)((radius - 10) * sin(rad));
    cv::line(frame, center, cv::Point(x, y), cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    
    // Отрисовка центральной точки
    cv::circle(frame, center, 4, cv::Scalar(255, 255, 255), -1, cv::LINE_AA);

    // Вывод текста
    cv::putText(frame, label, cv::Point(center.x - 25, center.y - radius - 10), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    
    char valStr[32];
    snprintf(valStr, sizeof(valStr), "%.0f", value);
    cv::putText(frame, valStr, cv::Point(center.x - 15, center.y + radius + 20), cv::FONT_HERSHEY_DUPLEX, 0.5, color, 1, cv::LINE_AA);
}

void Dashboard::drawLinearGauge(cv::Mat& frame, cv::Rect rect, double value, double maxVal, const std::string& label) {
    // Отрисовка контура
    cv::rectangle(frame, rect, cv::Scalar(100, 100, 100), 2, cv::LINE_AA);
    
    // Вычисление ширины заполнения
    double valClamped = std::max(0.0, std::min(value, maxVal));
    double fraction = valClamped / maxVal;
    int fillWidth = (int)(rect.width * fraction);
    
    // Цвет зависит от величины
    cv::Scalar color;
    if (fraction > 0.8) color = cv::Scalar(0, 0, 255); // Красный при > 80%
    else if (fraction > 0.6) color = cv::Scalar(0, 165, 255); // Оранжевый
    else color = cv::Scalar(0, 255, 0); // Зеленый
    
    // Заполнение
    if (fillWidth > 0) {
        cv::Rect fillRect(rect.x, rect.y, fillWidth, rect.height);
        cv::rectangle(frame, fillRect, color, -1, cv::LINE_AA);
    }
    
    // Вывод текста
    cv::putText(frame, label, cv::Point(rect.x, rect.y - 5), cv::FONT_HERSHEY_DUPLEX, 0.4, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    
    char valStr[32];
    snprintf(valStr, sizeof(valStr), "%.1f", value);
    cv::putText(frame, valStr, cv::Point(rect.x + rect.width + 10, rect.y + 15), cv::FONT_HERSHEY_DUPLEX, 0.4, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
}
