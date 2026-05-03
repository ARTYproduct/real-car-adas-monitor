#include "onnx_classifier.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <stdexcept>

ONNXClassifier::ONNXClassifier(const std::string& modelPath, const std::string& jsonPath)
    : memoryInfo_(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)) {
    
    // 1. Читаем JSON параметры
    parseJson(jsonPath);

    // 2. Инициализируем ONNX Runtime
    env_ = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "ONNXClassifier");

    Ort::SessionOptions sessionOptions;
    sessionOptions.SetIntraOpNumThreads(1);
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    try {
        session_ = std::make_unique<Ort::Session>(*env_, modelPath.c_str(), sessionOptions);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to load ONNX model: ") + e.what());
    }
}

void ONNXClassifier::parseJson(const std::string& jsonPath) {
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open JSON file: " + jsonPath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonStr = buffer.str();

    auto extractArray = [&](const std::string& key, std::array<float, 6>& outArray) {
        size_t pos = jsonStr.find("\"" + key + "\"");
        if (pos == std::string::npos) throw std::runtime_error("Key not found in JSON: " + key);
        pos = jsonStr.find("[", pos);
        for (int i = 0; i < 6; ++i) {
            pos++;
            size_t commaPos = jsonStr.find(",", pos);
            size_t bracketPos = jsonStr.find("]", pos);
            size_t endPos = std::min(commaPos, bracketPos);
            std::string valStr = jsonStr.substr(pos, endPos - pos);
            outArray[i] = std::stof(valStr);
            pos = endPos;
        }
    };

    extractArray("mean", mean_);
    extractArray("std", std_);

    size_t classesPos = jsonStr.find("\"classes\"");
    if (classesPos != std::string::npos) {
        classesPos = jsonStr.find("[", classesPos);
        size_t endClassesPos = jsonStr.find("]", classesPos);
        std::string classesSubStr = jsonStr.substr(classesPos, endClassesPos - classesPos + 1);
        size_t quote1 = 0;
        while ((quote1 = classesSubStr.find("\"", quote1)) != std::string::npos) {
            size_t quote2 = classesSubStr.find("\"", quote1 + 1);
            if (quote2 != std::string::npos) {
                classes_.push_back(classesSubStr.substr(quote1 + 1, quote2 - quote1 - 1));
                quote1 = quote2 + 1;
            } else {
                break;
            }
        }
    }
}

ClassificationResult ONNXClassifier::classify(const std::array<float, 6>& features) {
    if (!session_) {
        throw std::runtime_error("ONNX Session is not initialized!");
    }

    // 1. Нормализация (z-score)
    std::array<float, 6> inputTensorValues;
    for (size_t i = 0; i < 6; ++i) {
        inputTensorValues[i] = (features[i] - mean_[i]) / std_[i];
    }

    // 2. Подготовка входных данных
    std::vector<int64_t> inputDims = {1, 6}; // Batch size 1, 6 features
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo_, inputTensorValues.data(), inputTensorValues.size(), inputDims.data(), inputDims.size()
    );

    Ort::AllocatorWithDefaultOptions allocator;
    auto inputNamePtr = session_->GetInputNameAllocated(0, allocator);
    auto outputNamePtr = session_->GetOutputNameAllocated(0, allocator);

    const char* inputNames[] = {inputNamePtr.get()};
    const char* outputNames[] = {outputNamePtr.get()};

    // 3. Запуск модели
    auto outputTensors = session_->Run(
        Ort::RunOptions{nullptr}, inputNames, &inputTensor, 1, outputNames, 1
    );

    // 4. Обработка выхода
    float* floatArr = outputTensors.front().GetTensorMutableData<float>();
    std::array<float, 3> logits = {floatArr[0], floatArr[1], floatArr[2]};

    // Softmax
    float maxLogit = std::max({logits[0], logits[1], logits[2]});
    float sumExp = 0.0f;
    std::array<float, 3> probabilities;
    // Вычисляем экспоненты и их сумму
    for (int i = 0; i < 3; ++i) {
        probabilities[i] = std::exp(logits[i] - maxLogit);
        // maxLogit вычитается для стабильности вычислений
        sumExp += probabilities[i];
    }
    
    int bestLabel = 0;
    float maxProb = 0.0f;
    // Делим на сумму, чтобы получить вероятности от 0 до 1
    for (int i = 0; i < 3; ++i) {
        probabilities[i] /= sumExp;
        if (probabilities[i] > maxProb) {
            maxProb = probabilities[i];
            // Записываем индекс с максимальной вероятностью
            bestLabel = i;
        }
    }

    ClassificationResult result;
    result.label = bestLabel;
    result.confidence = maxProb;
    result.scores = probabilities;
    if (bestLabel >= 0 && bestLabel < classes_.size()) {
        result.labelStr = classes_[bestLabel];
    } else {
        result.labelStr = "UNKNOWN";
    }

    return result;
}
