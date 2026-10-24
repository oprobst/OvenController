#include "main_menu.h"

bool MainMenu::add(const MenuEntry& entry) {
    if (count_ >= config::kMaxMenuEntries) {
        return false;
    }
    entries_[count_++] = entry;
    return true;
}

void MainMenu::moveBy(int32_t delta) {
    if (count_ == 0) {
        return;
    }
    int32_t target = static_cast<int32_t>(cursor_) + delta;
    const int32_t last = static_cast<int32_t>(count_) - 1;
    if (target < 0) {
        target = 0;
    } else if (target > last) {
        target = last;
    }
    cursor_ = static_cast<size_t>(target);
}

const char* MainMenu::logName(MenuEntryId id) {
    switch (id) {
        case MenuEntryId::Heat:
            return "heat";
        case MenuEntryId::Profile:
            return "profile";
        case MenuEntryId::Setup:
            return "setup";
    }
    return "unknown";
}
