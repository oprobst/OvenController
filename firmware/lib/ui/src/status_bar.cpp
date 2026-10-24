#include "status_bar.h"

#include <cstring>

#include "screen_layout.h"

StatusBar::StatusBar(IGraphics& graphics) : graphics_(graphics) {}

void StatusBar::invalidate() {
    separatorDrawn_ = false;
    wifi_.drawn = false;
    time_.drawn = false;
    date_.drawn = false;
    temperature_.drawn = false;
    faultLine_.drawn = false;
}

void StatusBar::draw(const StatusTexts& texts) {
    using namespace layout;
    drawRegion(wifi_, texts.wifi, kStatusWifi.x, kStatusWifi.y, kStatusWifi.w, kStatusWifi.h,
               FontSize::Small, Align::Left);
    drawRegion(time_, texts.time, kStatusTime.x, kStatusTime.y, kStatusTime.w, kStatusTime.h,
               FontSize::Medium, Align::Right);
    drawRegion(date_, texts.date, kStatusDate.x, kStatusDate.y, kStatusDate.w, kStatusDate.h,
               FontSize::Small, Align::Left);
    drawRegion(temperature_, texts.temperature, kStatusTemperature.x, kStatusTemperature.y,
               kStatusTemperature.w, kStatusTemperature.h, FontSize::Medium, Align::Right);
    drawRegion(faultLine_, texts.faultLine, kFaultLine.x, kFaultLine.y, kFaultLine.w, kFaultLine.h,
               FontSize::Medium, Align::Center);
    if (!separatorDrawn_) {
        graphics_.fillRect(kSeparator.x, kSeparator.y, kSeparator.w, kSeparator.h, Color::Muted);
        separatorDrawn_ = true;
    }
}

void StatusBar::drawRegion(Region& region, const StatusText& text, int16_t x, int16_t y, uint16_t w,
                           uint16_t h, FontSize size, Align align) {
    if (region.drawn && region.shown.color == text.color &&
        std::strcmp(region.shown.text, text.text) == 0) {
        return;
    }
    graphics_.drawText(x, y, w, h, text.text, size, text.color, Color::Background, align);
    region.shown = text;
    region.drawn = true;
}
