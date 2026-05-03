#pragma once
#include <string>
#include <vector>
#include <array>

/**
 * @brief Telemetry record parsed from a single OBD-II CSV row.
 */
struct VehicleData {
    double timestamp; ///< Milliseconds since trip start.
    double speed;     ///< Vehicle speed, km/h.
    double rpm;       ///< Engine RPM.
    double throttle;  ///< Absolute throttle position, percent.
    double temp;      ///< Outdoor air temperature, °C.
    std::array<float, 6> features; ///< Normalised feature vector for the ONNX classifier.
};

/**
 * @brief Parses OBD-II telemetry data from a CSV file.
 *
 * Expected columns (0-indexed):
 *  2=Timestampms, 5=Speed, 7=RPM, 8=AbsLoad, 9=OAT,
 *  10=ShortTermFuelTrim, 17=log_MAF.
 */
class OBDParser {
public:
    /**
     * @brief Constructs the parser with the given CSV file path.
     * @param filePath Path to the OBD-II CSV file.
     */
    explicit OBDParser(const std::string& filePath) : filePath_(filePath) {}

    /**
     * @brief Parses the CSV and returns all records.
     * @return Vector of @c VehicleData records; empty on error.
     */
    std::vector<VehicleData> parse();

    /**
     * @brief Static helper that parses a CSV file and returns records.
     * @param filePath Path to the OBD-II CSV file.
     * @return Vector of @c VehicleData records; empty on error.
     */
    static std::vector<VehicleData> parseCSV(const std::string& filePath);

private:
    std::string filePath_;
};
