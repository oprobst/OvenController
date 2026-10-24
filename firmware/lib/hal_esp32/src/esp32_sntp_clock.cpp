#include "esp32_sntp_clock.h"

#include <Arduino.h>
#include <WiFi.h>
#include <esp_sntp.h>

#include <cstdio>
#include <ctime>

#include "app_config.h"

Esp32SntpClock::Esp32SntpClock(ILogger& logger) : logger_(logger) {}

void Esp32SntpClock::begin() {
    if (WiFi.getMode() == WIFI_OFF) {
        return;  // no network stack: SNTP must not be started
    }
    configTzTime(config::kTimeZonePosix, config::kNtpServer1, config::kNtpServer2);
    started_ = true;
}

void Esp32SntpClock::poll() {
    // sntp_get_sync_status() reports COMPLETED once per synchronization and then resets.
    if (!started_ || synced_ || sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
        return;
    }
    synced_ = true;
    const time_t seconds = time(nullptr);
    struct tm local {};
    localtime_r(&seconds, &local);
    char fields[48];
    std::snprintf(fields, sizeof(fields), "local=\"%04d-%02d-%02dT%02d:%02d:%02d\"",
                  local.tm_year + 1900, local.tm_mon + 1, local.tm_mday, local.tm_hour,
                  local.tm_min, local.tm_sec);
    logger_.log(LogLevel::Info, "time", "synced", fields);
}

WallClockTime Esp32SntpClock::now() const {
    const time_t seconds = time(nullptr);
    struct tm local {};
    localtime_r(&seconds, &local);
    const uint16_t year = static_cast<uint16_t>(local.tm_year + 1900);
    return WallClockTime{
        synced_ && year >= config::kMinValidYear, year,
        static_cast<uint8_t>(local.tm_mon + 1),   static_cast<uint8_t>(local.tm_mday),
        static_cast<uint8_t>(local.tm_hour),      static_cast<uint8_t>(local.tm_min)};
}
