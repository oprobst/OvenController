#pragma once

#include "i_clock.h"
#include "i_serial_out.h"
#include "i_system_info.h"
#include "i_watchdog.h"

/// IClock based on millis().
class Esp32Clock final : public IClock {
public:
    Milliseconds now() const override;
};

/// IWatchdog based on the ESP-IDF task watchdog (restarts the controller on timeout).
class Esp32Watchdog final : public IWatchdog {
public:
    void start() override;
    void feed() override;
};

/// ISerialOut on UART0 with an enlarged transmit buffer; drops data instead of blocking.
class Esp32SerialOut final : public ISerialOut {
public:
    /// Configures the transmit buffer and opens UART0. Call once before the first write.
    void begin();
    bool tryWrite(const char* data, size_t len) override;
};

/// ISystemInfo based on esp_reset_reason().
class Esp32SystemInfo final : public ISystemInfo {
public:
    const char* resetReason() const override;
};
