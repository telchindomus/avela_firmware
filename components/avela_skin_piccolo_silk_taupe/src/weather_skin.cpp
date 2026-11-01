#include "avela_skin_piccolo_silk_taupe/weather_skin.hpp"

#include <array>
#include <cstddef>

#include "avela_skin_piccolo_silk_taupe/palette.hpp"
#include "avela_skin_piccolo_silk_taupe/weather_icons.hpp"
#include "avela_skin_piccolo_silk_taupe/weather_layout.hpp"
#include "avela_skin_piccolo_silk_taupe/widgets.hpp"
#include "avela_strings/fixed_string.hpp"
#include "sdkconfig.h"

namespace avela::skin_piccolo_silk_taupe {
namespace {

constexpr const auto& weather = layout::weather;
constexpr int percent = 100;
using DegreesText = FixedString<capacity_for("H -999°")>;

const char* condition_name(WeatherCondition condition)
{
    switch (condition) {
    case WeatherCondition::clear:
        return "Clear";
    case WeatherCondition::partly_cloudy:
        return "Partly cloudy";
    case WeatherCondition::cloudy:
        return "Cloudy";
    case WeatherCondition::fog:
        return "Fog";
    case WeatherCondition::drizzle:
        return "Drizzle";
    case WeatherCondition::rain:
        return "Rain";
    case WeatherCondition::snow:
        return "Snow";
    case WeatherCondition::thunderstorm:
        return "Thunderstorm";
    }
    return "";
}

const char* weekday_name(int weekday)
{
    static constexpr std::array<const char*, 7> names{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    return names[static_cast<std::size_t>(weekday) % names.size()];
}

const char* wind_unit(MeasurementSystem units)
{
    return units == MeasurementSystem::imperial ? "mph" : "km/h";
}

DegreesText degrees(const char* prefix, int value)
{
    DegreesText text;
    text.append(prefix).append_number(value).append("°");
    return text;
}

// What the place line adds while the values aren't fresh.
const char* status_note(WeatherStatus status)
{
    switch (status) {
    case WeatherStatus::loading:
        return " · updating";
    case WeatherStatus::waiting_for_network:
        return " · offline";
    case WeatherStatus::failed:
    case WeatherStatus::place_not_found:
        return " · not updated";
    case WeatherStatus::ready:
    case WeatherStatus::not_configured:
        break;
    }
    return "";
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void WeatherSkin::set_weather(const WeatherReport& report)
{
    if (m_has_report_ && report.m_revision == m_report_.m_revision) {
        return;
    }
    m_report_ = report;
    m_has_report_ = true;
    render();
}

//--------------------------------------------------------------------------------------------------

void WeatherSkin::set_host_name(std::string_view host_name)
{
    if (keep_text(m_host_name_, host_name)) {
        render();
    }
}

//--------------------------------------------------------------------------------------------------

void WeatherSkin::draw()
{
    m_shown_ = true;
    render();
}

//--------------------------------------------------------------------------------------------------

void WeatherSkin::render() const
{
    if (!m_shown_) {
        return;
    }
    m_surface_.m_canvas.render(layout::content, palette::background, [&](Painter& painter) {
        if (m_report_.m_has_values) {
            draw_values(painter);
            draw_footer_divider(painter);
            draw_forecast(painter);
        } else {
            draw_message(painter);
        }
    });
}

//--------------------------------------------------------------------------------------------------

void WeatherSkin::draw_values(Painter& painter) const
{
    const Fonts& fonts = m_surface_.m_fonts;
    const WeatherReport& report = m_report_;

    draw_weather_icon(painter, weather.m_icon_center, weather.m_icon_size, report.m_condition, report.m_daytime,
                      palette::background);
    painter.text(degrees("", report.m_temperature).c_str(), weather.m_temperature.m_x, weather.m_temperature.m_y,
                 fonts.m_display.get(), palette::text);
    painter.text(condition_name(report.m_condition), weather.m_condition.m_x, weather.m_condition.m_y,
                 fonts.m_heading.get(), palette::text, lgfx::textdatum_t::middle_left);
    LabelText details;
    details.append("feels ")
        .append(degrees("", report.m_feels_like).view())
        .append(" · wind ")
        .append_number(report.m_wind_speed)
        .append(" ")
        .append(wind_unit(report.m_units));
    painter.text(details.c_str(), weather.m_details.m_x, weather.m_details.m_y, fonts.m_caption.get(),
                 palette::text_secondary, lgfx::textdatum_t::middle_left);

    painter.text(degrees("H ", report.m_high).c_str(), weather.m_column_x, weather.m_high_y, fonts.m_body_strong.get(),
                 palette::text, lgfx::textdatum_t::middle_left);
    painter.text(degrees("L ", report.m_low).c_str(), weather.m_column_x, weather.m_low_y, fonts.m_body_strong.get(),
                 palette::text_secondary, lgfx::textdatum_t::middle_left);
    LabelText place;
    place.append(report.m_place.view()).append(status_note(report.m_status));
    painter.clip(Rect{.m_x = weather.m_column_x,
                      .m_y = layout::content.m_y,
                      .m_width = weather.m_column_clip_right - weather.m_column_x,
                      .m_height = layout::content.m_height});
    painter.text(place.c_str(), weather.m_column_x, weather.m_place_y, fonts.m_body.get(), palette::text_secondary,
                 lgfx::textdatum_t::middle_left);
    painter.clear_clip();

    // humidity as a ring, in the volume bars' colours
    const auto ring_x = static_cast<float>(weather.m_ring_center.m_x);
    const auto ring_y = static_cast<float>(weather.m_ring_center.m_y);
    painter.ring(ring_x, ring_y, weather.m_ring_radius, weather.m_ring_thickness, 0.0f, full_turn_degrees,
                 palette::volume_track, palette::background);
    painter.ring(ring_x, ring_y, weather.m_ring_radius, weather.m_ring_thickness, twelve_o_clock_degrees,
                 full_turn_degrees * static_cast<float>(report.m_humidity_percent) / static_cast<float>(percent),
                 palette::accent, palette::background);
    FixedString<number_capacity<int>> humidity;
    humidity.append_number(report.m_humidity_percent);
    painter.text(humidity.c_str(), weather.m_ring_center.m_x, weather.m_ring_center.m_y, fonts.m_figure.get(),
                 palette::text, lgfx::textdatum_t::middle_center);
    painter.text("humidity %", weather.m_ring_caption.m_x, weather.m_ring_caption.m_y, fonts.m_caption.get(),
                 palette::text_secondary, lgfx::textdatum_t::middle_center);
}

//--------------------------------------------------------------------------------------------------

void WeatherSkin::draw_forecast(Painter& painter) const
{
    const Fonts& fonts = m_surface_.m_fonts;
    constexpr const auto& row = weather.m_forecast;
    int left = row.m_left;
    for (const ForecastDay& day : m_report_.m_forecast) {
        painter.text(weekday_name(day.m_weekday), left + row.m_weekday_x, row.m_center_y, fonts.m_body.get(),
                     palette::text_secondary, lgfx::textdatum_t::middle_left);
        draw_weather_icon(painter, Point{.m_x = left + row.m_icon_x, .m_y = row.m_center_y}, row.m_icon_size,
                          day.m_condition, true, palette::background);
        painter.text(degrees("", day.m_high).c_str(), left + row.m_temperature_right, row.m_center_y,
                     fonts.m_body_strong.get(), palette::text, lgfx::textdatum_t::middle_right);
        left += row.m_cell_width;
    }
}

//--------------------------------------------------------------------------------------------------

void WeatherSkin::draw_message(Painter& painter) const
{
    const Fonts& fonts = m_surface_.m_fonts;
    const char* headline = "No weather";
    LabelText hint;
    switch (m_report_.m_status) {
    case WeatherStatus::not_configured:
        headline = "No weather yet";
        hint.append("Set a place: http://").append(m_host_name_.view()).append(".local");
        break;
    case WeatherStatus::waiting_for_network:
        headline = "Waiting for Wi-Fi";
        hint.append("The weather comes once the radio is online");
        break;
    case WeatherStatus::loading:
    case WeatherStatus::ready:
        headline = "Loading weather…";
        hint.append(m_report_.m_place.view());
        break;
    case WeatherStatus::place_not_found:
        headline = "Place not found";
        hint.append("Check \"").append(m_report_.m_place.view()).append("\" on the setup page");
        break;
    case WeatherStatus::failed:
        hint.append("Open-Meteo didn't answer; trying again soon");
        break;
    }
    painter.text(headline, weather.m_message.m_x, weather.m_message.m_y, fonts.m_headline.get(), palette::text,
                 lgfx::textdatum_t::middle_left);
    painter.text(hint.c_str(), weather.m_message_hint.m_x, weather.m_message_hint.m_y, fonts.m_body.get(),
                 palette::text_secondary, lgfx::textdatum_t::middle_left);
}

}  // namespace avela::skin_piccolo_silk_taupe
