#pragma once
#include <opencv2/opencv.hpp>
#include "DMSMonitor.h"

/**
 * @brief Renders the Driver Monitoring System (DMS) overlay onto the shared canvas.
 *
 * Occupies the right 640×480 region of a 1280×480 canvas.
 * Draws the raw camera feed, face corner markers, eye/head status indicators,
 * and coloured alert banners when drowsiness or distraction is detected.
 */
class DMSHUD {
public:
    /** @brief Default constructor – no external resources required. */
    DMSHUD();

    /**
     * @brief Draws the DMS panel on the right half of @p canvas.
     *
     * @param canvas      Combined 1280×480 BGR frame (modified in-place).
     * @param cameraFrame Raw 640×480 BGR camera frame to embed.
     * @param state       Driver state returned by @c DMSMonitor::analyze().
     */
    void draw(cv::Mat& canvas, const cv::Mat& cameraFrame, const DriverState& state);
};
