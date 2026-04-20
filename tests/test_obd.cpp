#include <gtest/gtest.h>
#include "../src/OBDParser.hpp"

// Тест 1: Проверка на несуществующий файл
TEST(OBDParserTest, FileNotFound) {
    OBDParser parser("non_existent.csv");
    auto data = parser.parse();
    EXPECT_TRUE(data.empty());
}

// Тест 2: Проверка корректности парсинга
TEST(OBDParserTest, ParseCorrectness) {

    OBDParser parser("data/obd_data.csv");
    auto data = parser.parse();
    if (!data.empty()) {
        EXPECT_GT(data[0].speed, -1);
        EXPECT_GT(data[0].rpm, -1);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}