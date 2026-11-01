#include "avela_weather/setup/weather_section.hpp"

#include "avela_settings/settings.hpp"
#include "avela_weather/model/weather_settings.hpp"

namespace avela {
namespace {

using LocatedText = FixedString<capacity_for("found: ") + WeatherSettings::located_name_capacity>;

}  // namespace

//--------------------------------------------------------------------------------------------------

void WeatherSection::write_form(HtmlWriter& writer)
{
    WeatherSettings saved{};
    const bool has_saved = Settings::load(saved);
    const bool imperial = has_saved && saved.m_units == MeasurementSystem::imperial;

    writer.raw(R"(<label for="place">Place</label>)"
               R"(<input id="place" name="place" maxlength="40" placeholder="a city, e.g. Warsaw" value=")");
    if (has_saved) {
        writer.text(saved.m_place.view());
    }
    writer.raw(R"(">)");
    if (has_saved && !saved.m_place.view().empty()) {
        if (saved.m_located) {
            writer.raw("<p>Found: ").text(saved.m_located_name.view()).raw("</p>");
        } else {
            writer.raw("<p>Not looked up yet: the radio does that once it is on your network.</p>");
        }
    }
    writer.raw(R"(<label for="units">Units</label><select id="units" name="units">)");
    writer.raw(imperial
                   ? R"(<option value="metric">°C, km/h</option><option value="imperial" selected>°F, mph</option>)"
                   : R"(<option value="metric" selected>°C, km/h</option><option value="imperial">°F, mph</option>)");
    writer.raw("</select><p>Empty place: no weather.</p>");
}

//--------------------------------------------------------------------------------------------------

void WeatherSection::write_values(ValueWriter& values)
{
    WeatherSettings saved{};
    if (!Settings::load(saved)) {
        saved = WeatherSettings{};
    }
    values.value("place", saved.m_place.view());
    values.value("units", saved.m_units == MeasurementSystem::imperial ? "imperial" : "metric");
    if (saved.m_place.view().empty()) {
        values.note("no place set: no weather");
    } else if (saved.m_located) {
        LocatedText found;
        found.append("found: ").append(saved.m_located_name.view());
        values.note(found.view());
    } else {
        values.note("not looked up yet (the radio does that once it is online)");
    }
}

//--------------------------------------------------------------------------------------------------

ApplyResult WeatherSection::apply(const FormFields& submitted)
{
    const std::optional<std::string_view> place = submitted.find("place");
    const std::optional<std::string_view> units = submitted.find("units");
    if (place && place->size() > WeatherReport::place_capacity - 1) {
        return ApplyResult::invalid("place", "The place may have up to 40 characters.");
    }
    if (units && *units != "metric" && *units != "imperial") {
        return ApplyResult::invalid("units", "The units are metric or imperial.");
    }

    // a field that isn't given keeps its stored value (the console may change one at a time); read under the
    // settings' lock, so a save in between (the weather service's coordinates, the console) isn't undone
    WeatherSettings settings{};
    const bool saved = Settings::update<WeatherSettings>([&](WeatherSettings& stored) {
        settings = merged_settings(stored, place, units);
        stored = settings;
        return true;
    });
    if (!saved) {
        return ApplyResult::storage_failed();
    }
    ApplyResult result;
    result.m_saved = true;
    if (settings.m_place.view().empty()) {
        result.m_message.append("Saved: no weather.");
    } else {
        result.m_message.append("Saved. The weather screen shows ")
            .append(settings.m_place.view())
            .append(" in a moment.");
    }
    return result;
}

}  // namespace avela
