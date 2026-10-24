#include "welcome_screen.h"

#include <cstdio>

#include "screen_layout.h"

WelcomeScreen::WelcomeScreen(IGraphics& graphics) : graphics_(graphics) {}

void WelcomeScreen::draw(const FirmwareIdentity& identity) {
    using namespace layout;
    graphics_.drawText(kWelcomeGreeting.x, kWelcomeGreeting.y, kWelcomeGreeting.w,
                       kWelcomeGreeting.h, "Willkommen!", FontSize::Large, Color::Foreground,
                       Color::Background, Align::Center);
    graphics_.drawText(kWelcomeTitle.x, kWelcomeTitle.y, kWelcomeTitle.w, kWelcomeTitle.h,
                       "Ofen-Controller", FontSize::Medium, Color::Foreground, Color::Background,
                       Align::Center);

    char line[40];
    std::snprintf(line, sizeof(line), "Firmware %s", identity.version);
    graphics_.drawText(kWelcomeFirmware.x, kWelcomeFirmware.y, kWelcomeFirmware.w,
                       kWelcomeFirmware.h, line, FontSize::Small, Color::Muted, Color::Background,
                       Align::Center);
    std::snprintf(line, sizeof(line), "PCB REV %s", identity.pcbRevision);
    graphics_.drawText(kWelcomePcb.x, kWelcomePcb.y, kWelcomePcb.w, kWelcomePcb.h, line,
                       FontSize::Small, Color::Muted, Color::Background, Align::Center);
}
