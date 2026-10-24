#pragma once

#include <cstdint>

/// Log levels; a record is written if its level is less than or equal to the configured one.
enum class LogLevel : uint8_t { Error = 1, Warn = 2, Info = 3, Debug = 4 };

/// Structured logging; format see specs/001-startup-status-menu/contracts/log-format.md.
class ILogger {
public:
    virtual ~ILogger() = default;

    /// Writes one log record. Never blocks and never allocates.
    /// @param level severity
    /// @param module short module name without spaces, e.g. "temp"
    /// @param message free text, written in quotes
    /// @param fields optional additional key=value pairs (no leading space) or nullptr
    /// Records above the configured level, or that do not fit the output buffer, are dropped.
    virtual void log(LogLevel level, const char* module, const char* message,
                     const char* fields = nullptr) = 0;
};
