// A line console on the USB serial port (the console UART, the one ESP-IDF's log uses): reads what is typed, echoes
// it with backspace, and hands each finished line to its handler, on a task of its own (static stack). Output goes to
// stdout (the same port), so the log's lines and the console's share it; a reply holds stdout (OutputLock), so no log
// line lands inside it. Used by SetupConsole.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace avela {

class SerialConsole {
public:
    using LineHandler = void (*)(void* context, std::string_view line);

    // Holds the console's output for one whole reply: other tasks' log lines wait until it ends (stdout's own lock,
    // which ESP-IDF's log takes for each line too). It's recursive, so a log call inside doesn't deadlock, but its line
    // would land inside the reply and break it: don't log while holding it. Inside it, take no lock another task may
    // hold while it logs: reading Settings is fine (NVS doesn't log at the default level).
    class OutputLock {
    public:
        OutputLock() { flockfile(stdout); }
        OutputLock(const OutputLock&) = delete;
        OutputLock& operator=(const OutputLock&) = delete;
        OutputLock(OutputLock&&) = delete;
        OutputLock& operator=(OutputLock&&) = delete;
        ~OutputLock() { funlockfile(stdout); }
    };

    static constexpr const char* task_name = "console";
    static constexpr uint32_t stack_bytes = 4096;  // the sections' apply() runs here (Settings, NVS)

    SerialConsole(void* context, LineHandler handler) : m_context_(context), m_handler_(handler) {}
    SerialConsole(const SerialConsole&) = delete;
    SerialConsole& operator=(const SerialConsole&) = delete;
    SerialConsole(SerialConsole&&) = delete;
    SerialConsole& operator=(SerialConsole&&) = delete;
    ~SerialConsole() = default;

    // Takes over the console UART's input and starts reading. Once.
    void init();

    static void prompt();
    // Text to the console (stdout), at once.
    static void write(std::string_view text);

    static constexpr std::size_t line_capacity = 384;  // a station: name, genre and a 200-character address

private:
    static void run(void* self);
    void run();
    void take(char character);

    void* m_context_;
    LineHandler m_handler_;
    std::array<char, line_capacity> m_line_{};
    std::size_t m_length_ = 0;
    bool m_after_carriage_return_ = false;
    StaticTask_t m_task_{};
    std::array<StackType_t, stack_bytes / sizeof(StackType_t)> m_stack_{};
};

}  // namespace avela
