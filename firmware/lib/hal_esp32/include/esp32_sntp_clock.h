#pragma once

#include "i_logger.h"
#include "i_wall_clock.h"

/// IWallClock using the SNTP client of ESP-IDF with the CET/CEST time zone from app_config.h.
/// After the first synchronization the system clock keeps running without network.
class Esp32SntpClock final : public IWallClock {
public:
    /// @param logger diagnostics; must outlive the clock
    explicit Esp32SntpClock(ILogger& logger);

    /// Starts SNTP; does nothing if WiFi is off (not configured).
    void begin() override;
    void poll() override;
    WallClockTime now() const override;

private:
    ILogger& logger_;
    bool started_ = false;
    bool synced_ = false;
};
