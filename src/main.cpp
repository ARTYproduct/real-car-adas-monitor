#include <opencv2/opencv.hpp>
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <ctime>

#include "SharedState.h"
#include "OBDParser.hpp"
#include "onnx_classifier.h"
#include "Dashboard.h"
#include "DMSMonitor.h"
#include "DMSHUD.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&t), "%H:%M:%S");
    return ss.str();
}

static std::string screenshotPath(int idx) {
    std::ostringstream ss;
    ss << "output/screenshot_" << std::setw(3) << std::setfill('0') << idx << ".png";
    return ss.str();
}

// ---------------------------------------------------------------------------
// OBD thread – reads CSV at 10 Hz, classifies, writes SharedState
// ---------------------------------------------------------------------------

static void obdThread(SharedState& state,
                      const std::string& csvPath,
                      const std::string& modelPath,
                      const std::string& jsonPath)
{
    std::vector<VehicleData> records;
    try {
        records = OBDParser::parseCSV(csvPath);
    } catch (...) {
        std::cerr << "[OBD] Failed to parse CSV." << std::endl;
        state.running = false;
        return;
    }

    if (records.empty()) {
        std::cerr << "[OBD] No records found in CSV." << std::endl;
        state.running = false;
        return;
    }

    std::unique_ptr<ONNXClassifier> classifier;
    try {
        classifier = std::make_unique<ONNXClassifier>(modelPath, jsonPath);
    } catch (const std::exception& e) {
        std::cerr << "[OBD] ONNX init failed: " << e.what() << std::endl;
        state.running = false;
        return;
    }

    size_t idx = 0;
    constexpr auto period = std::chrono::milliseconds(100); // 10 Hz

    while (state.running) {
        auto tick = std::chrono::steady_clock::now();

        if (!state.paused) {
            const VehicleData& rec = records[idx % records.size()];
            ClassificationResult cls = classifier->classify(rec.features);

            {
                std::lock_guard<std::mutex> lock(state.mutex);
                state.currentOBD      = rec;
                state.classification  = cls;
                state.obdProcessed++;
            }

            idx++;
        }

        std::this_thread::sleep_until(tick + period);
    }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main() {
    // --- Ensure output directory exists ---
    std::filesystem::create_directories("output");

    std::cout << "[INFO] Initializing DMS models..." << std::endl;
    DMSMonitor dms("models/deploy.prototxt",
                   "models/res10_300x300_ssd_iter_140000.caffemodel",
                   "models/haarcascade_eye.xml");
    DMSHUD dmsHud;
    Dashboard dashboard;

    // --- Open webcam ---
    std::cout << "[INFO] Opening webcam..." << std::endl;
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "[ERROR] Cannot open webcam!" << std::endl;
        return -1;
    }

    // --- Video writer (output/result_situation2.mp4) ---
    constexpr int CANVAS_W = 960;
    constexpr int CANVAS_H = 480;
    cv::VideoWriter writer;
    writer.open("output/result_situation2.mp4",
                cv::VideoWriter::fourcc('m','p','4','v'),
                30.0,
                cv::Size(CANVAS_W, CANVAS_H));
    if (!writer.isOpened()) {
        std::cerr << "[WARN] Cannot open video writer. Recording disabled." << std::endl;
    }

    // --- Alert log ---
    std::ofstream alertLog("output/dms_alerts.log", std::ios::out | std::ios::trunc);
    alertLog << "# DMS Alerts Log – ADAS Monitor\n";
    alertLog << "# Format: [HH:MM:SS] TYPE | OBD_Style\n\n";

    // --- Shared state + OBD thread ---
    SharedState state;
    // Seed with defaults so the dashboard has something to draw from frame 1
    state.currentOBD.speed    = 0;
    state.currentOBD.rpm      = 800;
    state.currentOBD.temp     = 90;
    state.currentOBD.throttle = 0;
    state.classification.labelStr = "NORMAL";

    std::thread obd(obdThread,
                    std::ref(state),
                    "data/obd_data.csv",
                    "data/driver_classifier.onnx",
                    "data/normalization_params.json");

    // --- Timing ---
    auto startTime = std::chrono::steady_clock::now();

    int screenshotIdx = 0;
    int frameCount    = 0;

    std::cout << "[INFO] Running. Controls: Q=quit  SPACE=pause  S=screenshot" << std::endl;

    // --- Main loop ---
    while (state.running) {
        // 1. Read camera frame
        cv::Mat raw;
        cap >> raw;
        if (raw.empty()) {
            std::cerr << "[WARN] Empty frame – skipping." << std::endl;
            continue;
        }

        // 2. Pause handling
        int key = cv::waitKey(30) & 0xFF;
        if (key == 'q' || key == 'Q') {
            state.running = false;
            break;
        }
        if (key == ' ') {
            bool p = state.paused.load();
            state.paused.store(!p);
            std::cout << "[INFO] " << (p ? "Resumed." : "Paused.") << std::endl;
        }

        if (state.paused) {
            cv::putText(raw, "PAUSED", {20, 40},
                        cv::FONT_HERSHEY_DUPLEX, 1.2, {0, 255, 255}, 2, cv::LINE_AA);
            cv::imshow("ADAS Monitor", raw);
            continue;
        }

        // 3. Crop + resize camera frame to 640×480
        cv::Mat cropped;
        if (raw.cols * 3 > raw.rows * 4) {
            int w = raw.rows * 4 / 3;
            int x = (raw.cols - w) / 2;
            cropped = raw(cv::Rect(x, 0, w, raw.rows));
        } else {
            int h = raw.cols * 3 / 4;
            int y = (raw.rows - h) / 2;
            cropped = raw(cv::Rect(0, y, raw.cols, h));
        }
        cv::Mat camFrame;
        cv::resize(cropped, camFrame, cv::Size(640, 480));

        // 4. DMS analysis
        DriverState driverState = dms.analyze(camFrame);

        // 5. Read OBD data under mutex
        VehicleData obd;
        std::string style;
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            obd   = state.currentOBD;
            style = state.classification.labelStr;
        }

        // 6. Update alert counters
        bool alertFired = false;
        if (driverState.alert_drowsy) {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.alertDrowsy++;
            state.alertCount++;
            alertFired = true;
            alertLog << "[" << timestamp() << "] DROWSY       | " << style << "\n";
            alertLog.flush();
        }
        if (driverState.alert_distracted) {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.alertDistracted++;
            state.alertCount++;
            alertFired = true;
            alertLog << "[" << timestamp() << "] DISTRACTED   | " << style << "\n";
            alertLog.flush();
        }
        if (style == "AGGRESSIVE") {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.alertAggressive++;
            state.alertCount++;
            alertFired = true;
            alertLog << "[" << timestamp() << "] AGGRESSIVE   | " << style << "\n";
            alertLog.flush();
        }
        (void)alertFired;

        // 7. Compose 1280×480 canvas
        cv::Mat canvas = cv::Mat::zeros(CANVAS_H, CANVAS_W, CV_8UC3);

        // Left 320×480 – Dashboard
        cv::Mat leftROI = canvas(cv::Rect(0, 0, 320, 480));
        dashboard.draw(leftROI,
                       obd.speed, obd.rpm, obd.temp,
                       /* fuel placeholder */ 60.0,
                       obd.throttle, style);

        // Right 640×480 – DMS HUD (starts at x=320)
        dmsHud.draw(canvas, camFrame, driverState);

        // 8. Show & record
        cv::imshow("ADAS Monitor", canvas);
        if (writer.isOpened()) {
            writer.write(canvas);
        }

        // 9. Screenshot on 'S'
        if (key == 's' || key == 'S') {
            std::string path = screenshotPath(screenshotIdx++);
            cv::imwrite(path, canvas);
            std::cout << "[INFO] Screenshot saved: " << path << std::endl;
        }

        frameCount++;
    }

    // --- Cleanup ---
    state.running = false;
    if (obd.joinable()) obd.join();
    cap.release();
    if (writer.isOpened()) writer.release();
    alertLog.close();
    cv::destroyAllWindows();

    // --- Final statistics ---
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - startTime).count();

    int obdProcessed, alertCount, alertDrowsy, alertDistracted, alertAggressive;
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        obdProcessed    = state.obdProcessed;
        alertCount      = state.alertCount;
        alertDrowsy     = state.alertDrowsy;
        alertDistracted = state.alertDistracted;
        alertAggressive = state.alertAggressive;
    }

    std::cout << "\n========== ADAS Monitor – Session Statistics ==========\n";
    std::cout << "  Runtime            : " << elapsed << " s\n";
    std::cout << "  OBD records read   : " << obdProcessed << "\n";
    std::cout << "  Total alerts       : " << alertCount << "\n";
    std::cout << "    Drowsy           : " << alertDrowsy << "\n";
    std::cout << "    Distracted       : " << alertDistracted << "\n";
    std::cout << "    Aggressive drive : " << alertAggressive << "\n";
    std::cout << "=======================================================\n";

    return 0;
}
