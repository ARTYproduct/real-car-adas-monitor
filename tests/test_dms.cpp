#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include "../src/DMSMonitor.h"

// -----------------------------------------------------------------------
// Тест 1: По умолчанию модуль не загружен (плохие пути → модели не найдены)
//   Ожидаем, что конструктор не выбрасывает исключение и модуль
//   просто работает в «degraded» режиме: analyze() возвращает face_detected=false.
// -----------------------------------------------------------------------
TEST(DMSMonitorTest, NotLoadedWithBadPaths) {
    // Bad paths — конструктор выведет предупреждение, но не упадёт
    ASSERT_NO_THROW({
        DMSMonitor dms("bad_proto.prototxt",
                       "bad_model.caffemodel",
                       "bad_cascade.xml");
        cv::Mat frame = cv::Mat::zeros(480, 640, CV_8UC3);
        DriverState s = dms.analyze(frame);
        EXPECT_FALSE(s.face_detected);
    });
}

// -----------------------------------------------------------------------
// Тест 2: analyze() на пустом кадре не падает и возвращает face_detected=false
// -----------------------------------------------------------------------
TEST(DMSMonitorTest, AnalyzeEmptyFrameNoFace) {
    DMSMonitor dms("bad_proto.prototxt",
                   "bad_model.caffemodel",
                   "bad_cascade.xml");

    cv::Mat emptyFrame; // cv::Mat::empty() == true
    DriverState state;
    ASSERT_NO_THROW({ state = dms.analyze(emptyFrame); });
    EXPECT_FALSE(state.face_detected);
    EXPECT_FALSE(state.alert_drowsy);
    EXPECT_FALSE(state.alert_distracted);
}

// -----------------------------------------------------------------------
// Тест 3: Загрузка реальных моделей (если файлы присутствуют)
// -----------------------------------------------------------------------
TEST(DMSMonitorTest, LoadRealModels) {
    const std::string proto   = "models/deploy.prototxt";
    const std::string caffeM  = "models/res10_300x300_ssd_iter_140000.caffemodel";
    const std::string cascade = "models/haarcascade_eye.xml";

    // Пропускаем тест, если файлов нет (CI без моделей)
    if (!std::ifstream(proto).good()  ||
        !std::ifstream(caffeM).good() ||
        !std::ifstream(cascade).good())
    {
        GTEST_SKIP() << "Model files not found, skipping real-load test.";
    }

    ASSERT_NO_THROW({
        DMSMonitor dms(proto, caffeM, cascade);
        // Чёрный кадр: нет лица → face_detected должен быть false
        cv::Mat black = cv::Mat::zeros(480, 640, CV_8UC3);
        DriverState s = dms.analyze(black);
        EXPECT_FALSE(s.face_detected);
    });
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
