#pragma once

#include <string>
#include <vector>

#include "i_logger.h"

/// Records every log call.
class FakeLogger final : public ILogger {
public:
    struct Entry {
        LogLevel level;
        std::string module;
        std::string message;
        std::string fields;
    };

    void log(LogLevel level, const char* module, const char* message,
             const char* fields = nullptr) override {
        entries.push_back({level, module, message, fields != nullptr ? fields : ""});
    }

    size_t count(LogLevel level, const std::string& message) const {
        size_t n = 0;
        for (const Entry& entry : entries) {
            if (entry.level == level && entry.message == message) {
                ++n;
            }
        }
        return n;
    }

    const Entry* find(const std::string& message) const {
        for (const Entry& entry : entries) {
            if (entry.message == message) {
                return &entry;
            }
        }
        return nullptr;
    }

    std::vector<Entry> entries;
};
