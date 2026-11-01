// The weather settings section (setup page and serial console): the place (a city name, looked up with Open-Meteo
// once the radio is online) and the units. Saving stores WeatherSettings; the weather service sees the new revision
// and updates. Shows what the lookup found for the saved place. Runs on the web server's or the console's task: reads
// and writes only Settings.
#pragma once

#include <array>
#include <string_view>

#include "avela_setup/apply_result.hpp"
#include "avela_setup/setup_field.hpp"
#include "avela_setup/setup_section_concept.hpp"

namespace avela {

class WeatherSection {
public:
    static constexpr std::string_view id = "weather";
    static constexpr std::string_view title = "Weather";
    static constexpr std::array fields{
        SetupField{.m_name = "place", .m_description = "a city, e.g. Warsaw (empty: no weather)", .m_secret = false},
        SetupField{.m_name = "units", .m_description = "metric (°C, km/h) or imperial (°F, mph)", .m_secret = false},
    };

    static void write_form(HtmlWriter& writer);
    static ApplyResult apply(const FormFields& submitted);
    static void write_values(ValueWriter& values);
};

static_assert(SetupSectionConcept<WeatherSection>);

}  // namespace avela
