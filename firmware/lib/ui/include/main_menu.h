#pragma once

#include <cstddef>
#include <cstdint>

#include "app_config.h"

/// Stable identifiers of main menu entries.
enum class MenuEntryId : uint8_t { Heat, Profile, Setup };

/// One selectable main menu entry.
struct MenuEntry {
    MenuEntryId id;
    const char* label;  ///< UTF-8 text shown on screen; must stay valid
};

/// Main menu model: fixed-size list of entries and the cursor. Entries are added in the
/// composition root, so new features add entries without changing this class (FR-012).
class MainMenu {
public:
    /// Appends an entry.
    /// @return false if config::kMaxMenuEntries entries already exist
    bool add(const MenuEntry& entry);

    /// Moves the cursor by delta entries; stops at the first and last entry (no wrap-around).
    void moveBy(int32_t delta);

    /// @return id of the marked entry; the cursor is unchanged. Requires at least one entry.
    MenuEntryId select() const { return entries_[cursor_].id; }

    size_t count() const { return count_; }
    size_t cursor() const { return cursor_; }

    /// @param index 0 .. count() - 1
    const MenuEntry& entry(size_t index) const { return entries_[index]; }

    /// @return log name of an entry id, e.g. "heat".
    static const char* logName(MenuEntryId id);

private:
    MenuEntry entries_[config::kMaxMenuEntries] = {};
    size_t count_ = 0;
    size_t cursor_ = 0;
};
