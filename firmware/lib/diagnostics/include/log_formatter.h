#pragma once

#include <cstddef>
#include <cstdint>

#include "app_config.h"
#include "i_logger.h"

/// Formats log records as `t=<ms> lvl=<LEVEL> mod=<module> msg="<text>"[ <key>=<value>]*\n`.
class LogFormatter {
public:
    /// Required size of the output buffer: maximum line length plus terminator.
    static constexpr size_t kBufferSize = config::kLogLineMaxChars + 1;

    /// Formats one record. Lines longer than config::kLogLineMaxChars (including the newline)
    /// are cut and marked with ` trunc=1`.
    /// @param out output buffer, at least kBufferSize bytes; always null-terminated
    /// @param outSize size of out in bytes
    /// @param timeMs time since start in ms
    /// @param fields optional key=value pairs or nullptr
    /// @return number of characters written, excluding the terminator (0 if out is too small)
    static size_t format(char* out, size_t outSize, uint32_t timeMs, LogLevel level,
                         const char* module, const char* message, const char* fields);

    /// @return "ERROR", "WARN", "INFO" or "DEBUG".
    static const char* levelName(LogLevel level);
};
