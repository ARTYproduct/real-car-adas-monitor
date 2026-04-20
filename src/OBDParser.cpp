#include "OBDParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

OBDParser::OBDParser(const std::string& filename) : filename(filename) {}

std::vector<VehicleData> OBDParser::parse() {
    std::vector<VehicleData> data;
    std::ifstream file(filename);
    if (!file.is_open()) return data;

    std::string line;
    std::getline(file, line); // Header

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string word;
        std::vector<std::string> row;
        while (std::getline(ss, word, ',')) {
            row.push_back(word);
        }

        if (row.size() > 9) {
            try {
                VehicleData entry;
                entry.timestamp = std::stod(row[2]);
                entry.speed = static_cast<int>(std::stod(row[5]));
                entry.rpm = static_cast<int>(std::stod(row[7]));
                entry.throttle = std::stod(row[8]);
                entry.temp = std::stod(row[9]);
                data.push_back(entry);
            } catch (...) { continue; }
        }
    }
    return data;
}