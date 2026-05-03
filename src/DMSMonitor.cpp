#include "DMSMonitor.h"
#include <iostream>

DMSMonitor::DMSMonitor(const std::string& facePrototxt, const std::string& faceModel, const std::string& eyeCascadePath) {
    try {
        faceNet_ = cv::dnn::readNetFromCaffe(facePrototxt, faceModel);
        if (!eyeCascade_.load(eyeCascadePath)) {
            std::cerr << "[ERROR] Could not load Haar cascade for eyes!" << std::endl;
        }
    } catch (const cv::Exception& e) {
        std::cerr << "[ERROR] DNN Face detector init failed: " << e.what() << std::endl;
    }
}

DriverState DMSMonitor::analyze(const cv::Mat& frame) {
    DriverState state;
    state.face_detected = false;
    state.eyes_open = false;
    state.looking_forward = true;
    state.eye_openness = 0.0f;
    state.head_turn_deg = 0.0f;
    state.alert_drowsy = false;
    state.alert_distracted = false;
    state.face_rect = cv::Rect(0, 0, 0, 0);

    // Guard: unloaded model or empty frame – return safe defaults
    if (faceNet_.empty() || frame.empty()) {
        return state;
    }

    // 1. Detect face
    cv::Rect faceRect = detectFace(frame);
    if (faceRect.area() == 0) {
        // No face detected
        return state;
    }

    state.face_detected = true;
    state.face_rect = faceRect;

    // 3. Estimate eye openness and detect eyes
    cv::Rect eyeROI(faceRect.x, faceRect.y, faceRect.width, faceRect.height / 2);
    eyeROI &= cv::Rect(0, 0, frame.cols, frame.rows);
    
    std::vector<cv::Rect> eyes;
    if (eyeROI.area() > 0) {
        cv::Mat faceUpperHalf = frame(eyeROI);
        eyeCascade_.detectMultiScale(faceUpperHalf, eyes, 1.1, 3, 0, cv::Size(10, 10));
    }

    state.eyes_open = !eyes.empty();
    state.eye_openness = state.eyes_open ? 1.0f : 0.0f;

    // 4. Estimate head turn based on eyes position inside the face
    float target_turn = estimateHeadTurn(faceRect, eyes);
    if (eyes.empty()) {
        target_turn = currentHeadTurn_; // При моргании сохраняем предыдущий угол
    }
    
    // Экспоненциальное сглаживание для устранения мелкого дрожания
    currentHeadTurn_ = currentHeadTurn_ * 0.7f + target_turn * 0.3f;
    state.head_turn_deg = currentHeadTurn_;

    bool is_turned = (std::abs(state.head_turn_deg) > 15.0f);
    distractionHistory_.push_back(is_turned);
    if (distractionHistory_.size() > 10) distractionHistory_.pop_front();

    int turnedCount = 0;
    for (bool t : distractionHistory_) {
        if (t) turnedCount++;
    }

    // Алерт и статус меняются только если голова повернута стабильно (6 из 10 кадров)
    if (turnedCount >= 6) {
        state.looking_forward = false;
        state.alert_distracted = true;
    } else {
        state.looking_forward = true;
        state.alert_distracted = false;
    }

    // 5. Track eye history (max 15 frames)
    eyesHistory_.push_back(state.eyes_open);
    if (eyesHistory_.size() > 15) {
        eyesHistory_.pop_front();
    }

    // 6. Drowsiness alert logic: eyes closed in 10 out of 15 last frames
    int closedCount = 0;
    for (bool open : eyesHistory_) {
        if (!open) closedCount++;
    }
    if (closedCount >= 10 && eyesHistory_.size() == 15) {
        state.alert_drowsy = true;
    }

    return state;
}

cv::Rect DMSMonitor::detectFace(const cv::Mat& frame) {
    cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, cv::Size(300, 300), cv::Scalar(104.0, 177.0, 123.0), false, false);
    faceNet_.setInput(blob);
    cv::Mat detections = faceNet_.forward();

    cv::Mat detectionMat(detections.size[2], detections.size[3], CV_32F, detections.ptr<float>());

    cv::Rect bestFace(0, 0, 0, 0);
    float maxConfidence = 0.0f;

    for (int i = 0; i < detectionMat.rows; i++) {
        float confidence = detectionMat.at<float>(i, 2);

        if (confidence > 0.5f) {
            int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
            int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
            int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
            int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);

            cv::Rect object(xLeftBottom, yLeftBottom, xRightTop - xLeftBottom, yRightTop - yLeftBottom);
            
            // Ensure within frame bounds
            object &= cv::Rect(0, 0, frame.cols, frame.rows);

            if (confidence > maxConfidence) {
                maxConfidence = confidence;
                bestFace = object;
            }
        }
    }
    return bestFace;
}

float DMSMonitor::estimateHeadTurn(const cv::Rect& faceRect, const std::vector<cv::Rect>& eyes) {
    if (eyes.empty()) {
        return 0.0f; // Can't tell if head is turned without eyes
    }
    
    // Find average X center of detected eyes
    float eyesCenterX = 0.0f;
    for (const auto& eye : eyes) {
        eyesCenterX += eye.x + eye.width / 2.0f;
    }
    eyesCenterX /= eyes.size(); // This is relative to the eyeROI (which starts at faceRect.x)
    
    // The center of the face box relative to its own left edge is width / 2
    float faceCenterX = faceRect.width / 2.0f;
    
    // Normalized offset [-1.0, 1.0]
    float offset = (eyesCenterX - faceCenterX) / faceCenterX;
    
    // Convert to rough degrees. A 25% shift is about 15 degrees.
    return offset * 60.0f;
}
