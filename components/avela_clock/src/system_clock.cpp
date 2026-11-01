#include "avela_clock/system_clock.hpp"

#include <chrono>
#include <cstdlib>
#include <ctime>

#include "avela_settings/settings.hpp"

namespace avela {
namespace {

constexpr int first_tm_year = 1900;  // struct tm counts years from 1900 and months from 0

}  // namespace

//--------------------------------------------------------------------------------------------------

void SystemClock::init()
{
    apply_time_zone();
}

//--------------------------------------------------------------------------------------------------

void SystemClock::update()
{
    if (Settings::revision<ClockSettings>() != m_applied_revision_) {
        apply_time_zone();
    }
}

//--------------------------------------------------------------------------------------------------

ClockSettings SystemClock::settings()
{
    ClockSettings settings{};
    if (!Settings::load(settings) || settings.m_time_zone.empty()) {
        settings = ClockSettings{};
        settings.m_time_zone.append(CONFIG_AVELA_TIME_ZONE);
    }
    return settings;
}

//--------------------------------------------------------------------------------------------------

void SystemClock::apply_time_zone()
{
    m_applied_revision_ = Settings::revision<ClockSettings>();  // before loading: a save in between applies next frame
    setenv("TZ", settings().m_time_zone.c_str(), 1);
    tzset();
}

//--------------------------------------------------------------------------------------------------

LocalTime SystemClock::now()
{
    const bool synchronised = m_synchronised_.load();  // before reading the time: a set in between counts next time
    const std::time_t seconds = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm local{};
    if (localtime_r(&seconds, &local) == nullptr) {
        return LocalTime{.m_synchronised = synchronised};
    }
    return LocalTime{
        .m_year = local.tm_year + first_tm_year,
        .m_month = local.tm_mon + 1,
        .m_day = local.tm_mday,
        .m_weekday = local.tm_wday,
        .m_hour = local.tm_hour,
        .m_minute = local.tm_min,
        .m_second = local.tm_sec,
        .m_synchronised = synchronised,
    };
}

}  // namespace avela
