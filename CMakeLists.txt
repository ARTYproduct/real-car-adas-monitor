cmake_minimum_required(VERSION 3.20)
project(RealCarMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем CMake, где искать Homebrew библиотеки на M1
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

# Поиск OpenCV (стандартный способ)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Для ONNX Runtime на Mac через brew
include_directories(/opt/homebrew/include/onnxruntime)
link_directories(/opt/homebrew/lib)

# Далее по списку...
file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(RealCarMonitor ${SOURCES})
target_link_libraries(RealCarMonitor ${OpenCV_LIBS} onnxruntime)