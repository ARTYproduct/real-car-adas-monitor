#include "OBDParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<VehicleData> OBDParser::parse() {
    return parseCSV(filePath_);
}

std::vector<VehicleData> OBDParser::parseCSV(const std::string& filePath) {
    std::vector<VehicleData> data;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "[ERROR] OBD-II Data not found at: " << filePath << std::endl;
        return data;
    }

    std::string line;
    std::getline(file, line); // Пропускаем Header

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string value;
        std::vector<std::string> tokens;
        
        while (std::getline(ss, value, ',')) {
            tokens.push_back(value);
        }

        // Если не хватает колонок (по формату Kaggle их ~19)
        if (tokens.size() < 18) continue;

        try {
            VehicleData frame;
            frame.timestamp = std::stod(tokens[2]);
            frame.speed = std::stod(tokens[5]);
            frame.rpm = std::stod(tokens[7]);
            frame.throttle = std::stod(tokens[8]);
            frame.temp = std::stod(tokens[9]);
            
            // Фичи для нейросети (6 штук)
            frame.features[0] = static_cast<float>(frame.speed);
            frame.features[1] = static_cast<float>(frame.rpm);
            frame.features[2] = static_cast<float>(frame.throttle);
            frame.features[3] = static_cast<float>(frame.temp);
            frame.features[4] = std::stof(tokens[10]); // Short_Term_Fuel_Trim
            frame.features[5] = std::stof(tokens[17]); // log_MAF

            data.push_back(frame);
        } catch (...) {
            // Игнорируем мусорные строки
            continue;
        }
    }

    file.close();
    std::cout << "[SUCCESS] Parsed " << data.size() << " frames for ADAS Monitor." << std::endl;
    return data;
}