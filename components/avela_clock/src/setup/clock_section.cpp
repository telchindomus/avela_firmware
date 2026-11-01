#include "avela_clock/setup/clock_section.hpp"

#include <algorithm>

#include "avela_clock/model/clock_settings.hpp"
#include "avela_clock/model/time_zones.hpp"
#include "avela_clock/system_clock.hpp"
#include "avela_settings/settings.hpp"

namespace avela {
namespace {

bool printable(std::string_view text)
{
    constexpr char first_printable = ' ';
    constexpr char last_printable = '~';
    return std::ranges::all_of(
        text, [](char character) { return character >= first_printable && character <= last_printable; });
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void ClockSection::write_form(HtmlWriter& writer)
{
    const ClockSettings saved = SystemClock::settings();
    const std::string_view current = saved.m_time_zone.view();
    const bool named =
        std::ranges::any_of(named_time_zones, [&](const NamedTimeZone& zone) { return zone.m_posix == current; });
    writer.raw(R"(<label for="time_zone">Time zone</label><select id="time_zone" name="time_zone">)");
    if (!named) {
        // set on the console: kept until another is chosen
        writer.raw(R"(<option value=")").text(current).raw(R"(" selected>)").text(current).raw("</option>");
    }
    for (const NamedTimeZone& zone : named_time_zones) {
        writer.raw(R"(<option value=")").text(zone.m_posix).raw(zone.m_posix == current ? R"(" selected>)" : R"(">)");
        writer.text(zone.m_name).raw("</option>");
    }
    writer.raw("</select><p>Daylight saving time follows by itself. Another zone: on the serial console, as a POSIX "
               "TZ string.</p>");
}

//--------------------------------------------------------------------------------------------------

void ClockSection::write_values(ValueWriter& values)
{
    values.value("time_zone", SystemClock::settings().m_time_zone.view());
}

//--------------------------------------------------------------------------------------------------

ApplyResult ClockSection::apply(const FormFields& submitted)
{
    const std::optional<std::string_view> time_zone = submitted.find("time_zone");
    if (!time_zone) {
        return ApplyResult::invalid("", "Nothing to do: give time_zone.");
    }
    if (time_zone->empty() || time_zone->size() > ClockSettings::time_zone_capacity - 1 || !printable(*time_zone)) {
        return ApplyResult::invalid("time_zone", "A POSIX TZ string has 1 to 63 characters.");
    }
    ClockSettings settings{};
    settings.m_time_zone.append(*time_zone);
    if (!Settings::save(settings)) {
        return ApplyResult::storage_failed();
    }
    ApplyResult result;
    result.m_saved = true;
    result.m_message.append("Time zone set: ").append(*time_zone).append(".");
    return result;
}

}  // namespace avela
