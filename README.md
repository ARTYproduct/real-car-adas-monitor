# Real-Car ADAS Monitor

Real-time Advanced Driver-Assistance System combining OBD-II telemetry analysis, driving-style classification via a neural network, and a Driver Monitoring System (DMS) using computer vision.

## What the system does

The application runs two threads in parallel:

| Thread | Role |
|--------|------|
| **OBD thread** (10 Hz) | Reads an OBD-II CSV dataset, classifies driving style (NORMAL / AGGRESSIVE / ECO) with an ONNX neural network, and writes results to `SharedState` |
| **Main / render thread** | Captures webcam frames, analyses the driver's face and eyes via DMS, composes a 1280×480 HUD, records video, and logs alerts |

The 1280×480 output frame is split into two panels:
- **Left 640×480** – Dashboard with speedometer, tachometer, temperature, fuel, and throttle gauges
- **Right 640×480** – DMS camera feed with face tracking, eye / head-turn status, and alert overlays

## Technology stack

| Component | Technology |
|-----------|-----------|
| Language | C++17 |
| Build system | CMake 3.20+ |
| Computer vision | OpenCV 4.x (DNN, VideoCapture, VideoWriter) |
| Face detection | Caffe SSD ResNet-10 (res10_300x300) |
| Eye detection | OpenCV Haar cascade |
| Driving-style classifier | ONNX Runtime + custom MLP model |
| Unit tests | Google Test (GTest) |
| Documentation | Doxygen |

## Building

```bash
# 1. Clone and enter the repo
git clone <repo-url> && cd real-car-adas-monitor

# 2. Create a build directory
cmake -B build -S .

# 3. Compile
cmake --build build -j$(nproc)
```

Prerequisites (macOS / Homebrew):
```bash
brew install opencv onnxruntime googletest doxygen
```

## Running

```bash
# From the build directory
cd build
./RealCarMonitor
```

**Keyboard controls:**

| Key | Action |
|-----|--------|
| `Q` | Quit and print statistics |
| `SPACE` | Pause / resume |
| `S` | Save screenshot to `output/` |

Output files (created automatically in `build/output/`):
- `result_situation2.mp4` – recorded session video
- `dms_alerts.log` – timestamped alert log
- `screenshot_XXX.png` – screenshots saved with `S`

## Running tests

```bash
cd build
ctest --output-on-failure
# or run individual test binaries:
./run_tests       # OBD parser tests
./run_dms_tests   # DMS monitor tests
```

## Generating documentation

```bash
# From the project root
doxygen Doxyfile
# Then open:
open docs/doxygen/html/index.html
```

## Results

| Metric | Value |
|--------|-------|
| Render FPS | ~30 fps (1280×480) |
| OBD update rate | 10 Hz |
| Face detection confidence threshold | 0.5 |
| Drowsiness alert trigger | Eyes closed in ≥ 10 / 15 frames |
| Distraction alert trigger | Head turned in ≥ 6 / 10 frames |
| Alert types | Drowsy, Distracted, Aggressive driving |

## Project structure

```
real-car-adas-monitor/
├── src/
│   ├── main.cpp            # Entry point, OBD thread, main loop
│   ├── SharedState.h       # Thread-safe shared data structure
│   ├── Dashboard.{h,cpp}   # Instrument cluster rendering
│   ├── DMSMonitor.{h,cpp}  # Driver state analysis
│   ├── DMSHUD.{h,cpp}      # DMS overlay rendering
│   ├── OBDParser.{hpp,cpp} # CSV telemetry parser
│   └── onnx_classifier.{h,cpp}  # ONNX driving-style classifier
├── tests/
│   ├── test_obd.cpp        # OBD parser unit tests
│   └── test_dms.cpp        # DMS monitor unit tests
├── data/                   # ONNX model + normalisation params
├── models/                 # Caffe & Haar cascade model files
├── docs/                   # Architecture docs + generated Doxygen
├── Doxyfile                # Doxygen configuration
└── CMakeLists.txt
```
