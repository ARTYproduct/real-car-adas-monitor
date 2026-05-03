#pragma once
#include <string>
#include <vector>
#include <array>

struct VehicleData {
    double timestamp;
    double speed;
    double rpm;
    double throttle;
    double temp;
    std::array<float, 6> features; // Для ONNXClassifier
};

class OBDParser {
public:
    OBDParser(const std::string& filePath) : filePath_(filePath) {}
    std::vector<VehicleData> parse();

    static std::vector<VehicleData> parseCSV(const std::string& filePath);

private:
    std::string filePath_;
};