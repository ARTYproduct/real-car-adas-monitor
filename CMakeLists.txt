cmake_minimum_required(VERSION 3.20)
project(RealCarMonitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Homebrew libraries on Apple Silicon
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

# OpenCV
find_package(OpenCV REQUIRED)

# ONNX Runtime
include_directories(/opt/homebrew/include/onnxruntime)
link_directories(/opt/homebrew/lib)

include_directories(src)
include_directories(${OpenCV_INCLUDE_DIRS})

# -----------------------------------------------------------------------
# Main executable
# -----------------------------------------------------------------------
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp" "src/*.h")

add_executable(RealCarMonitor ${SOURCES})

target_link_libraries(RealCarMonitor
    ${OpenCV_LIBS}
    onnxruntime
)

# Copy data/models to build directory; create output folder
add_custom_command(TARGET RealCarMonitor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:RealCarMonitor>/data"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/models" "$<TARGET_FILE_DIR:RealCarMonitor>/models"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:RealCarMonitor>/output"
)

# -----------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------
enable_testing()

include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

find_package(GTest REQUIRED)

# OBD parser tests
add_executable(run_tests tests/test_obd.cpp src/OBDParser.cpp)
target_link_libraries(run_tests
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME OBDTests COMMAND run_tests)

# DMS monitor tests
add_executable(run_dms_tests
    tests/test_dms.cpp
    src/DMSMonitor.cpp
)
target_include_directories(run_dms_tests PRIVATE src ${OpenCV_INCLUDE_DIRS})
target_link_libraries(run_dms_tests
    GTest::gtest
    GTest::gtest_main
    ${OpenCV_LIBS}
)

# Copy models for DMS tests so real-model test can find them
add_custom_command(TARGET run_dms_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/models" "$<TARGET_FILE_DIR:run_dms_tests>/models"
)

add_test(NAME DMSTests COMMAND run_dms_tests)
