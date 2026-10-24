#include "scheduler.h"

bool Scheduler::add(TaskFn fn, void* context, uint32_t periodMs) {
    if (fn == nullptr || count_ >= kMaxTasks) {
        return false;
    }
    tasks_[count_++] = Task{fn, context, periodMs, Milliseconds{0}, false};
    return true;
}

void Scheduler::tick(Milliseconds now) {
    for (size_t i = 0; i < count_; ++i) {
        Task& task = tasks_[i];
        if (!task.hasRun || now.elapsedSince(task.lastRun) >= task.periodMs) {
            task.lastRun = now;
            task.hasRun = true;
            task.fn(task.context, now);
        }
    }
}
