#pragma once

#include <cstddef>

/// Non-blocking serial output.
class ISerialOut {
public:
    virtual ~ISerialOut() = default;

    /// Writes len bytes if they fit completely into the transmit buffer.
    /// @param data bytes to send, not necessarily null-terminated
    /// @param len number of bytes
    /// @return true if written, false if discarded because the buffer is full. Never blocks.
    virtual bool tryWrite(const char* data, size_t len) = 0;
};
