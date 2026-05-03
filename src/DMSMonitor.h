#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <deque>

struct DriverState {
    bool face_detected;
    bool eyes_open;
    bool looking_forward;
    float eye_openness;
    float head_turn_deg;
    bool alert_drowsy;
    bool alert_distracted;
    cv::Rect face_rect;
};

class DMSMonitor {
public:
    DMSMonitor(const std::string& facePrototxt, const std::string& faceModel, const std::string& eyeCascadePath);
    
    DriverState analyze(const cv::Mat& frame);

private:
    cv::dnn::Net faceNet_;
    cv::CascadeClassifier eyeCascade_;
    std::deque<bool> eyesHistory_; // max size 15
    std::deque<bool> distractionHistory_; // max size 10
    float currentHeadTurn_ = 0.0f;

    cv::Rect detectFace(const cv::Mat& frame);
    float estimateHeadTurn(const cv::Rect& faceRect, const std::vector<cv::Rect>& eyes);
};
