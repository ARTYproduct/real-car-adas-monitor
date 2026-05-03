#pragma once

// Это каркас для одной строки данных из машины
struct ObdFrame {
    double timestamp;   // Время
    double speed;       // Скорость (км/ч)
    double rpm;         // Обороты двигателя (об/мин)
    double throttle;    // Положение педали газа (%)
};