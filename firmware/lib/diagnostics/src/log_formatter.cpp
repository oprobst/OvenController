#include "log_formatter.h"

#include <cstdio>

namespace {

constexpr char kTruncMarker[] = " trunc=1";
constexpr size_t kTruncMarkerLength = sizeof(kTruncMarker) - 1;

/// Appends characters up to a fixed capacity and remembers whether anything was cut.
class Appender {
public:
    Appender(char* buffer, size_t capacity) : buffer_(buffer), capacity_(capacity) {}

    void put(char c) {
        if (length_ < capacity_) {
            buffer_[length_++] = c;
        } else {
            overflow_ = true;
        }
    }

    void put(const char* text) {
        for (; *text != '\0'; ++text) {
            put(*text);
        }
    }

    void putEscaped(const char* text) {
        for (; *text != '\0'; ++text) {
            if (*text == '"' || *text == '\\') {
                put('\\');
            }
            put(*text);
        }
    }

    void putUnsigned(uint32_t value) {
        char digits[11];
        std::snprintf(digits, sizeof(digits), "%lu", static_cast<unsigned long>(value));
        put(digits);
    }

    size_t length() const { return length_; }
    void truncate(size_t length) { length_ = length; }
    bool overflow() const { return overflow_; }

private:
    char* buffer_;
    size_t capacity_;
    size_t length_ = 0;
    bool overflow_ = false;
};

}  // namespace

size_t LogFormatter::format(char* out, size_t outSize, uint32_t timeMs, LogLevel level,
                            const char* module, const char* message, const char* fields) {
    if (out == nullptr || outSize < kBufferSize) {
        if (out != nullptr && outSize > 0) {
            out[0] = '\0';
        }
        return 0;
    }

    // Content without the trailing newline.
    const size_t contentCapacity = config::kLogLineMaxChars - 1;
    Appender line(out, contentCapacity);
    line.put("t=");
    line.putUnsigned(timeMs);
    line.put(" lvl=");
    line.put(levelName(level));
    line.put(" mod=");
    line.put(module != nullptr ? module : "");
    line.put(" msg=\"");
    line.putEscaped(message != nullptr ? message : "");
    line.put('"');
    if (fields != nullptr && fields[0] != '\0') {
        line.put(' ');
        line.put(fields);
    }

    size_t length = line.length();
    if (line.overflow()) {
        length = contentCapacity - kTruncMarkerLength;
        for (size_t i = 0; i < kTruncMarkerLength; ++i) {
            out[length++] = kTruncMarker[i];
        }
    }
    out[length++] = '\n';
    out[length] = '\0';
    return length;
}

const char* LogFormatter::levelName(LogLevel level) {
    switch (level) {
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Debug:
            return "DEBUG";
    }
    return "?";
}
