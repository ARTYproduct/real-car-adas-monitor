#pragma once

#include <mutex>
#include <atomic>
#include "OBDParser.hpp"
#include "onnx_classifier.h"

/**
 * @brief Shared state exchanged between the OBD thread and the main (render) thread.
 *
 * Access to @c currentOBD and @c classification must be protected by @c mutex.
 * @c running and @c paused are lock-free and may be read/written from any thread.
 */
struct SharedState {
    /** @brief Latest OBD telemetry record written by the OBD thread. */
    VehicleData currentOBD{};

    /** @brief Latest driving-style classification produced by the OBD thread. */
    ClassificationResult classification{};

    /** @brief Total number of driver alerts (drowsy + distracted + aggressive). */
    int alertCount = 0;

    /** @brief Per-category alert counters. */
    int alertDrowsy      = 0;
    int alertDistracted  = 0;
    int alertAggressive  = 0;

    /** @brief Number of OBD records processed so far. */
    int obdProcessed = 0;

    /** @brief Set to false to signal both threads to stop. */
    std::atomic<bool> running{true};

    /** @brief When true the main loop skips rendering (SPACE key). */
    std::atomic<bool> paused{false};

    /** @brief Protects currentOBD, classification, and alert counters. */
    std::mutex mutex;
};
