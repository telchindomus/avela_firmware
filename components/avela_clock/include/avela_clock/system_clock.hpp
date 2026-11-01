// The system clock as the app sees it. The ESP32 has no battery-backed clock: the system clock ticks from
// power-on, starting at 1 January 1970, until something sets it. Who sets it: ESP-IDF's SNTP only (no setter here;
// docs/developer/ARCHITECTURE.md: Clock); the network service, which runs SNTP, then calls mark_synchronised().
// All static: there is one system clock per device, so there is one "has it been set" flag, and every task (and
// the SNTP callback, a plain C function) reaches the same one. The time zone: the one saved on the setup page or the
// console (ClockSettings), else menuconfig's (Avela clock); a newly saved one applies on the next frame.
#pragma once

#include <atomic>
#include <cstdint>

#include "avela_clock/local_time.hpp"
#include "avela_clock/model/clock_settings.hpp"

namespace avela {

class SystemClock {
public:
    // Applies the time zone. Once, at startup, after Settings::init().
    static void init();
    // Main loop, every frame: applies a time zone saved since.
    static void update();
    // The time zone in force: the saved one, else menuconfig's. Any task.
    [[nodiscard]] static ClockSettings settings();

    // The system clock now, as local time, and whether it has been set: both from one call.
    [[nodiscard]] static LocalTime now();

    // The system clock has been set (by SNTP); safe from any task.
    static void mark_synchronised() { m_synchronised_.store(true); }

private:
    static void apply_time_zone();

    static inline std::atomic<bool> m_synchronised_{false};  // one system clock, one flag
    static inline uint32_t m_applied_revision_ = 0;          // of ClockSettings; main loop only
};

}  // namespace avela
