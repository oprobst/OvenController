#pragma once

#include <cstddef>
#include <cstdint>

#include "units.h"

/// Cooperative scheduler with a fixed table of periodic tasks. Runs inside the main loop and
/// never blocks. A task runs on the first tick after it was added and then whenever its period
/// has elapsed; missed periods are not caught up.
class Scheduler {
public:
    /// Task callback.
    /// @param context pointer passed to add()
    /// @param now current time
    using TaskFn = void (*)(void* context, Milliseconds now);

    static constexpr size_t kMaxTasks = 8;

    /// Adds a periodic task.
    /// @param periodMs period in ms; 0 = run on every tick
    /// @return false if the table is full or fn is nullptr
    bool add(TaskFn fn, void* context, uint32_t periodMs);

    /// Runs all due tasks in the order they were added.
    void tick(Milliseconds now);

private:
    struct Task {
        TaskFn fn;
        void* context;
        uint32_t periodMs;
        Milliseconds lastRun;
        bool hasRun;
    };

    Task tasks_[kMaxTasks] = {};
    size_t count_ = 0;
};
