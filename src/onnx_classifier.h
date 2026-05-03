#pragma once

#include <string>
#include <array>
#include <vector>
#include <memory>
#include <onnxruntime_cxx_api.h>

/**
 * @brief Output of the ONNX driving-style classifier.
 */
struct ClassificationResult {
    int label;                    ///< Predicted class index (0=NORMAL, 1=AGGRESSIVE, 2=...).
    float confidence;             ///< Probability of the winning class [0, 1].
    std::array<float, 3> scores;  ///< Softmax probabilities for all three classes.
    std::string labelStr;         ///< Human-readable label string.
};

/**
 * @brief Runs a pre-trained ONNX model to classify driving style from OBD features.
 *
 * The model expects a 6-element z-score-normalised input vector:
 * [speed, rpm, throttle, temp, fuelTrim, log_MAF].
 */
class ONNXClassifier {
public:
    /**
     * @brief Loads the ONNX model and normalisation parameters from disk.
     * @param modelPath  Path to the @c .onnx model file.
     * @param jsonPath   Path to the JSON file with @c mean and @c std arrays.
     * @throws std::runtime_error if the model or JSON cannot be loaded.
     */
    ONNXClassifier(const std::string& modelPath, const std::string& jsonPath);

    /**
     * @brief Classifies a single feature vector.
     * @param features  Raw (unnormalised) feature vector of length 6.
     * @return @c ClassificationResult with the predicted label and scores.
     */
    ClassificationResult classify(const std::array<float, 6>& features);

private:
    void parseJson(const std::string& jsonPath);

    std::unique_ptr<Ort::Env>     env_;
    std::unique_ptr<Ort::Session> session_;
    Ort::MemoryInfo memoryInfo_{nullptr};

    std::array<float, 6>     mean_;
    std::array<float, 6>     std_;
    std::vector<std::string> classes_;
};
