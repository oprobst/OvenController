#pragma once

#include <string>
#include <vector>

#include "i_serial_out.h"

/// Records written lines; rejects writes that exceed the remaining capacity.
class FakeSerialOut final : public ISerialOut {
public:
    bool tryWrite(const char* data, size_t len) override {
        if (len > capacity) {
            ++rejected;
            return false;
        }
        writes.emplace_back(data, len);
        return true;
    }

    size_t capacity = 4096;
    size_t rejected = 0;
    std::vector<std::string> writes;
};
