#pragma once

/// Hardware watchdog (constitution principle IVa).
class IWatchdog {
public:
    virtual ~IWatchdog() = default;

    /// Activates the watchdog with config::kWatchdogTimeoutMs. Call once during setup.
    virtual void start() = 0;

    /// Must be called at least once per timeout, otherwise the controller restarts.
    virtual void feed() = 0;
};
