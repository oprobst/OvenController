#include "logger.h"

#include <cstdio>

#include "log_formatter.h"

Logger::Logger(ISerialOut& out, const IClock& clock, LogLevel maxLevel)
    : out_(out), clock_(clock), maxLevel_(maxLevel) {}

void Logger::log(LogLevel level, const char* module, const char* message, const char* fields) {
    if (static_cast<uint8_t>(level) > static_cast<uint8_t>(maxLevel_)) {
        return;
    }
    if (dropped_ > 0) {
        char countField[24];
        std::snprintf(countField, sizeof(countField), "count=%lu",
                      static_cast<unsigned long>(dropped_));
        if (!write(LogLevel::Warn, "log", "dropped", countField)) {
            ++dropped_;
            return;
        }
        dropped_ = 0;
    }
    if (!write(level, module, message, fields)) {
        ++dropped_;
    }
}

bool Logger::write(LogLevel level, const char* module, const char* message, const char* fields) {
    char line[LogFormatter::kBufferSize];
    const size_t length = LogFormatter::format(line, sizeof(line), clock_.now().value, level,
                                               module, message, fields);
    return out_.tryWrite(line, length);
}
