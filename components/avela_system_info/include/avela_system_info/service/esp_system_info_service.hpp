// Reports the device's state from ESP-IDF: internal heap, the app description's version and the timer's uptime. Logs
// once a minute how much of its stack each task has used at most: the stacks are sized from these numbers (RAM is
// scarce).
#pragma once

#include <cstdint>
#include <span>

#include "avela_system_info/service/system_info_service_concept.hpp"
#include "avela_system_info/service/task_stack.hpp"

namespace avela {

class EspSystemInfoService {
public:
    // tasks: every task whose stack is logged, with its declared size (static storage)
    explicit EspSystemInfoService(std::span<const TaskStack> tasks) : m_tasks_(tasks) {}

    void update(SystemInfoModel& model);

private:
    static constexpr int64_t stacks_log_interval_us = 60'000'000;

    // "used/size" of every task's stack, one line (tag "system"); a task that isn't running is left out.
    void log_stack_use() const;

    std::span<const TaskStack> m_tasks_;
    int64_t m_stacks_logged_us_ = 0;
};

static_assert(SystemInfoServiceConcept<EspSystemInfoService>);

}  // namespace avela
