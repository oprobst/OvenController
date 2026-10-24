#include <unity.h>

#include <cstring>
#include <string>

#include "log_formatter.h"

void setUp() {}
void tearDown() {}

namespace {

std::string format(uint32_t timeMs, LogLevel level, const char* module, const char* message,
                   const char* fields) {
    char buffer[LogFormatter::kBufferSize];
    const size_t length =
        LogFormatter::format(buffer, sizeof(buffer), timeMs, level, module, message, fields);
    TEST_ASSERT_EQUAL_size_t(std::strlen(buffer), length);
    return std::string(buffer, length);
}

}  // namespace

void test_formats_record_without_fields() {
    TEST_ASSERT_EQUAL_STRING("t=12 lvl=INFO mod=sys msg=\"boot\"\n",
                             format(12, LogLevel::Info, "sys", "boot", nullptr).c_str());
}

void test_formats_record_with_fields() {
    TEST_ASSERT_EQUAL_STRING(
        "t=5250 lvl=WARN mod=temp msg=\"sensor fault\" code=E101\n",
        format(5250, LogLevel::Warn, "temp", "sensor fault", "code=E101").c_str());
}

void test_level_names() {
    TEST_ASSERT_EQUAL_STRING("ERROR", LogFormatter::levelName(LogLevel::Error));
    TEST_ASSERT_EQUAL_STRING("WARN", LogFormatter::levelName(LogLevel::Warn));
    TEST_ASSERT_EQUAL_STRING("INFO", LogFormatter::levelName(LogLevel::Info));
    TEST_ASSERT_EQUAL_STRING("DEBUG", LogFormatter::levelName(LogLevel::Debug));
}

void test_escapes_quotes_and_backslashes_in_message() {
    TEST_ASSERT_EQUAL_STRING("t=1 lvl=DEBUG mod=ui msg=\"a \\\"b\\\" c\\\\d\"\n",
                             format(1, LogLevel::Debug, "ui", "a \"b\" c\\d", nullptr).c_str());
}

void test_truncates_long_lines_to_255_chars_and_marks_them() {
    const std::string longMessage(400, 'x');
    const std::string line = format(1, LogLevel::Info, "sys", longMessage.c_str(), nullptr);
    TEST_ASSERT_EQUAL_size_t(255, line.size());
    TEST_ASSERT_EQUAL_CHAR('\n', line.back());
    TEST_ASSERT_EQUAL_STRING(" trunc=1\n", line.substr(line.size() - 9).c_str());
}

void test_line_of_exactly_maximum_length_is_not_truncated() {
    // Prefix 't=1 lvl=INFO mod=sys msg="' has 26 chars, closing quote 1, newline 1 -> 227 x.
    const std::string message(227, 'x');
    const std::string line = format(1, LogLevel::Info, "sys", message.c_str(), nullptr);
    TEST_ASSERT_EQUAL_size_t(255, line.size());
    TEST_ASSERT_TRUE(line.find("trunc=1") == std::string::npos);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_formats_record_without_fields);
    RUN_TEST(test_formats_record_with_fields);
    RUN_TEST(test_level_names);
    RUN_TEST(test_escapes_quotes_and_backslashes_in_message);
    RUN_TEST(test_truncates_long_lines_to_255_chars_and_marks_them);
    RUN_TEST(test_line_of_exactly_maximum_length_is_not_truncated);
    return UNITY_END();
}
