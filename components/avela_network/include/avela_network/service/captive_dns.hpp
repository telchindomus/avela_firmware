// A DNS server for the setup network: every name resolves to the radio itself, so a phone that joins the setup
// network finds the setup page whatever it asks for (captive portal). Runs on its own small task (static stack);
// listens on port 53 only while enabled.
#pragma once

#include <array>
#include <atomic>
#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace avela {

class CaptiveDns {
public:
    static constexpr const char* task_name = "captive_dns";
    static constexpr uint32_t stack_bytes = 3072;

    // Starts the task. Once.
    void init();

    // Answers every query with `address` (IPv4, network byte order) from now on. Any task.
    void enable(uint32_t address) { m_address_.store(address); }
    void disable() { m_address_.store(0); }

private:
    static void run(void* self);
    void serve();

    std::atomic<uint32_t> m_address_{0};  // 0 = disabled
    StaticTask_t m_task_{};
    std::array<StackType_t, stack_bytes / sizeof(StackType_t)> m_stack_{};
};

}  // namespace avela
