// The clock settings section (setup page and serial console): the time zone. The page offers the named zones
// (model/time_zones.hpp); the console takes any POSIX TZ string. Saving stores ClockSettings; the clock applies it on
// the main loop (SystemClock::update). Runs on the web server's or the console's task: reads and writes only
// Settings.
#pragma once

#include <array>
#include <string_view>

#include "avela_setup/apply_result.hpp"
#include "avela_setup/setup_field.hpp"
#include "avela_setup/setup_section_concept.hpp"

namespace avela {

class ClockSection {
public:
    static constexpr std::string_view id = "clock";
    static constexpr std::string_view title = "Clock";
    static constexpr std::array fields{
        SetupField{.m_name = "time_zone",
                   .m_description = R"(a POSIX TZ string, e.g. "CET-1CEST,M3.5.0,M10.5.0/3")",
                   .m_secret = false},
    };

    static void write_form(HtmlWriter& writer);
    static ApplyResult apply(const FormFields& submitted);
    static void write_values(ValueWriter& values);
};

static_assert(SetupSectionConcept<ClockSection>);

}  // namespace avela
