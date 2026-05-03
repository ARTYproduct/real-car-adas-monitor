#pragma once

#include <string>
#include <array>
#include <vector>
#include <memory>
#include <onnxruntime_cxx_api.h>

struct ClassificationResult {
    int label;
    float confidence;
    std::array<float, 3> scores;
    std::string labelStr;
};

class ONNXClassifier {
public:
    ONNXClassifier(const std::string& modelPath, const std::string& jsonPath);
    ClassificationResult classify(const std::array<float, 6>& features);

private:
    void parseJson(const std::string& jsonPath);
    
    std::unique_ptr<Ort::Env> env_;
    std::unique_ptr<Ort::Session> session_;
    Ort::MemoryInfo memoryInfo_{nullptr};

    std::array<float, 6> mean_;
    std::array<float, 6> std_;
    std::vector<std::string> classes_;
};
