#pragma once
#include <opencv2/opencv.hpp>
#include <string>

/**
 * @brief Renders a real-time instrument cluster (dashboard) onto an OpenCV frame.
 *
 * The dashboard occupies the left 640×480 region of the combined canvas and
 * displays speedometer, tachometer, temperature, fuel level, and throttle gauges
 * along with a colour-coded driving-style label.
 */
class Dashboard {
public:
    /** @brief Default constructor – no external resources required. */
    Dashboard();

    /**
     * @brief Draws all dashboard instruments onto @p frame.
     *
     * @param frame        Target OpenCV Mat (must be at least 640×480, CV_8UC3).
     * @param speed        Vehicle speed, km/h.
     * @param rpm          Engine RPM.
     * @param temp         Coolant / OAT temperature, °C.
     * @param fuel         Fuel level, percent [0, 100].
     * @param throttle     Throttle position, percent [0, 100].
     * @param drivingStyle Driving-style label string (e.g. "NORMAL", "AGGRESSIVE").
     */
    void draw(cv::Mat& frame,
              double speed, double rpm, double temp,
              double fuel, double throttle,
              const std::string& drivingStyle);

private:
    /**
     * @brief Draws a single circular analogue gauge.
     * @param frame   Target image.
     * @param center  Centre point of the gauge.
     * @param radius  Radius in pixels.
     * @param value   Current reading.
     * @param maxVal  Maximum scale value.
     * @param label   Text label drawn below the gauge.
     * @param color   Arc colour.
     */
    void drawGauge(cv::Mat& frame, cv::Point center, int radius,
                   double value, double maxVal,
                   const std::string& label, const cv::Scalar& color);

    /**
     * @brief Draws a horizontal bar gauge.
     * @param frame   Target image.
     * @param rect    Bounding rectangle for the bar.
     * @param value   Current reading.
     * @param maxVal  Maximum scale value.
     * @param label   Text label drawn above the bar.
     */
    void drawLinearGauge(cv::Mat& frame, cv::Rect rect,
                         double value, double maxVal,
                         const std::string& label);
};
