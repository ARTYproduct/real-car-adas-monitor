cmake_minimum_required(VERSION 3.20)
project(RealCarMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем CMake, где искать Homebrew библиотеки на M1
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

# Поиск OpenCV
find_package(OpenCV REQUIRED)

# Поиск ONNX Runtime (уже прописано через пути, оставляем как есть)
include_directories(/opt/homebrew/include/onnxruntime)
link_directories(/opt/homebrew/lib)

# Включаем заголовки из папки src (чтобы можно было писать #include "OBDParser.hpp" без лишних путей)
include_directories(src)
include_directories(${OpenCV_INCLUDE_DIRS})

# Собираем все исходники
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp" "src/*.h")

add_executable(RealCarMonitor ${SOURCES})

# Линковка
target_link_libraries(RealCarMonitor 
    ${OpenCV_LIBS} 
    onnxruntime
)

# Копируем папку data в директорию, где лежит бинарник, после сборки
add_custom_command(TARGET RealCarMonitor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:RealCarMonitor>/data"
)

# --- Секция тестирования ---
enable_testing()

# Явно указываем пути для GTest на Apple Silicon
include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

# Пытаемся найти пакет
find_package(GTest REQUIRED)

# Создаем исполняемый файл для тестов
add_executable(run_tests tests/test_obd.cpp src/OBDParser.cpp)

# Линкуем
target_link_libraries(run_tests 
    GTest::gtest 
    GTest::gtest_main
)

add_test(NAME OBDTests COMMAND run_tests)