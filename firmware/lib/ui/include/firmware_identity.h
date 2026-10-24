#pragma once

/// Firmware version and PCB revision shown on the welcome screen (principle VI).
struct FirmwareIdentity {
    const char* version;      ///< e.g. "0.1.0"
    const char* pcbRevision;  ///< e.g. "DEV"
};
