#pragma once
#include <string>
#include <vector>

struct VehicleData {
    double timestamp;
    int speed;
    int rpm;
    double throttle;
    double temp;
    double fuel;
};

class OBDParser {
public:
    OBDParser(const std::string& filename);
    std::vector<VehicleData> parse();
private:
    std::string filename;
};