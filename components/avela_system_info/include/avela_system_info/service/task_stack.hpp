// A task's stack as its owner declares it: the task's name and its stack size. main lists every task's for the
// system info service, which logs how much of each the task has used (the numbers stacks are sized from).
#pragma once

#include <cstdint>

namespace avela {

struct TaskStack {
    const char* m_name;
    uint32_t m_bytes;
};

}  // namespace avela
