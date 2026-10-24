#pragma once

/// Information about the last restart.
class ISystemInfo {
public:
    virtual ~ISystemInfo() = default;

    /// @return short name of the reset reason: "power_on", "brownout", "task_wdt", "int_wdt",
    ///         "panic", "sw", "ext" or "unknown". Never nullptr.
    virtual const char* resetReason() const = 0;
};
