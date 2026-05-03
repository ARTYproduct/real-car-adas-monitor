#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <deque>

/**
 * @brief Snapshot of the driver's state produced by @c DMSMonitor::analyze().
 */
struct DriverState {
    bool  face_detected;    ///< True if a face was found in the frame.
    bool  eyes_open;        ///< True if at least one eye is detected.
    bool  looking_forward;  ///< True if the head is roughly centred.
    float eye_openness;     ///< Eye openness score in [0, 1].
    float head_turn_deg;    ///< Estimated head yaw in degrees (positive = right).
    bool  alert_drowsy;     ///< True when drowsiness is detected (eyes closed ≥10/15 frames).
    bool  alert_distracted; ///< True when distraction is detected (head turned ≥6/10 frames).
    cv::Rect face_rect;     ///< Bounding box of the detected face in image coordinates.
};

/**
 * @brief Driver Monitoring System that analyses a single camera frame.
 *
 * Uses a Caffe SSD face detector and a Haar-cascade eye detector.
 * Maintains internal history buffers to debounce drowsiness and distraction alerts.
 */
class DMSMonitor {
public:
    /**
     * @brief Constructs the monitor and loads the required models from disk.
     *
     * If any model file is missing, a warning is printed and face/eye detection
     * will silently fail (returning default @c DriverState values).
     *
     * @param facePrototxt    Path to the Caffe deploy prototxt.
     * @param faceModel       Path to the Caffe caffemodel weights.
     * @param eyeCascadePath  Path to the Haar-cascade XML for eye detection.
     */
    DMSMonitor(const std::string& facePrototxt,
               const std::string& faceModel,
               const std::string& eyeCascadePath);

    /**
     * @brief Analyses one frame and returns the driver's current state.
     *
     * Safe to call with an empty frame; will return @c face_detected = false.
     *
     * @param frame  BGR image (any size; 640×480 recommended).
     * @return @c DriverState populated with detection results and alert flags.
     */
    DriverState analyze(const cv::Mat& frame);

private:
    cv::dnn::Net           faceNet_;
    cv::CascadeClassifier  eyeCascade_;
    std::deque<bool>       eyesHistory_;        ///< Rolling window of 15 frames for drowsiness.
    std::deque<bool>       distractionHistory_; ///< Rolling window of 10 frames for distraction.
    float                  currentHeadTurn_ = 0.0f;

    cv::Rect detectFace(const cv::Mat& frame);
    float    estimateHeadTurn(const cv::Rect& faceRect, const std::vector<cv::Rect>& eyes);
};
