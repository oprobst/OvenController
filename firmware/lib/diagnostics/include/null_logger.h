#pragma once

#include "i_logger.h"

/// Logger that discards everything; used when the build flag OVEN_LOG_LEVEL is 0.
class NullLogger final : public ILogger {
public:
    void log(LogLevel /*level*/, const char* /*module*/, const char* /*message*/,
             const char* /*fields*/ = nullptr) override {}
};
