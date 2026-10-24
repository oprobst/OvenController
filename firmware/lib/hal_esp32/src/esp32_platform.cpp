#include "esp32_platform.h"

#include <Arduino.h>
#include <esp_system.h>
#include <esp_task_wdt.h>

#include "app_config.h"

Milliseconds Esp32Clock::now() const { return Milliseconds{static_cast<uint32_t>(millis())}; }

void Esp32Watchdog::start() {
    // Reconfigures the task watchdog already started by ESP-IDF and adds the loop task.
    esp_task_wdt_init(config::kWatchdogTimeoutMs / 1000, true);
    esp_task_wdt_add(nullptr);
}

void Esp32Watchdog::feed() { esp_task_wdt_reset(); }

void Esp32SerialOut::begin() {
    Serial.setTxBufferSize(config::kSerialTxBufferBytes);
    Serial.begin(config::kSerialBaud);
}

bool Esp32SerialOut::tryWrite(const char* data, size_t len) {
    if (static_cast<size_t>(Serial.availableForWrite()) < len) {
        return false;
    }
    Serial.write(reinterpret_cast<const uint8_t*>(data), len);
    return true;
}

const char* Esp32SystemInfo::resetReason() const {
    switch (esp_reset_reason()) {
        case ESP_RST_POWERON:
            return "power_on";
        case ESP_RST_BROWNOUT:
            return "brownout";
        case ESP_RST_TASK_WDT:
            return "task_wdt";
        case ESP_RST_INT_WDT:
        case ESP_RST_WDT:
            return "int_wdt";
        case ESP_RST_PANIC:
            return "panic";
        case ESP_RST_SW:
            return "sw";
        case ESP_RST_EXT:
            return "ext";
        default:
            return "unknown";
    }
}
