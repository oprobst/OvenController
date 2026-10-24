#pragma once

#include <cstdint>

#include "i_clock.h"
#include "i_logger.h"
#include "i_serial_out.h"

#ifndef OVEN_LOG_LEVEL
#define OVEN_LOG_LEVEL 3
#endif

/// Log level selected at compile time via the build flag OVEN_LOG_LEVEL (1 = ERROR ... 4 = DEBUG).
/// OVEN_LOG_LEVEL=0 disables logging; the composition root then uses NullLogger.
constexpr LogLevel kCompiledLogLevel =
    static_cast<LogLevel>(OVEN_LOG_LEVEL > 0 ? OVEN_LOG_LEVEL : 1);

/// Logger writing formatted records to a non-blocking serial output.
/// Records that do not fit into the transmit buffer are dropped and counted; the next record
/// that can be written is preceded by `mod=log msg="dropped" count=<n>`.
class Logger final : public ILogger {
public:
    /// @param out serial output; must outlive the logger
    /// @param clock time source for the record timestamp; must outlive the logger
    /// @param maxLevel records with a higher level are discarded
    Logger(ISerialOut& out, const IClock& clock, LogLevel maxLevel = kCompiledLogLevel);

    void log(LogLevel level, const char* module, const char* message,
             const char* fields = nullptr) override;

    /// @return number of records dropped since the last successful drop report.
    uint32_t droppedCount() const { return dropped_; }

private:
    bool write(LogLevel level, const char* module, const char* message, const char* fields);

    ISerialOut& out_;
    const IClock& clock_;
    LogLevel maxLevel_;
    uint32_t dropped_ = 0;
};
