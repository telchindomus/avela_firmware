#include "avela_weather/model/weather_settings.hpp"

#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <string_view>

namespace avela {
namespace {

constexpr int32_t warsaw_latitude = 52'229'770;
constexpr int32_t warsaw_longitude = 21'011'780;

WeatherSettings saved_place(std::string_view place)
{
    WeatherSettings settings;
    settings.m_place.append(place);
    return settings;
}

WeatherSettings found(std::string_view place)
{
    WeatherSettings settings = saved_place(place);
    settings.m_located = true;
    settings.m_latitude_micro_degrees = warsaw_latitude;
    settings.m_longitude_micro_degrees = warsaw_longitude;
    settings.m_located_name.append("Warsaw, Poland");
    return settings;
}

TEST_CASE("take_location stores the coordinates for the place that was looked up")
{
    WeatherSettings saved = saved_place("Warsaw");
    saved.m_units = MeasurementSystem::imperial;  // saved during the lookup: kept
    REQUIRE(take_location(saved, found("Warsaw")));
    CHECK(saved.m_located);
    CHECK(saved.m_latitude_micro_degrees == warsaw_latitude);
    CHECK(saved.m_longitude_micro_degrees == warsaw_longitude);
    CHECK(saved.m_located_name.view() == "Warsaw, Poland");
    CHECK(saved.m_units == MeasurementSystem::imperial);
}

TEST_CASE("take_location leaves a place saved during the lookup alone")
{
    WeatherSettings saved = saved_place("Krakow");
    CHECK_FALSE(take_location(saved, found("Warsaw")));
    CHECK_FALSE(saved.m_located);
    CHECK(saved.m_latitude_micro_degrees == 0);
    CHECK(saved.m_located_name.view().empty());
}

TEST_CASE("take_location needs a found place and an unlocated record")
{
    WeatherSettings saved = saved_place("Warsaw");
    CHECK_FALSE(take_location(saved, saved_place("Warsaw")));
    CHECK_FALSE(saved.m_located);

    WeatherSettings located = found("Warsaw");
    located.m_located_name.clear().append("Warsaw, Masovia");
    CHECK_FALSE(take_location(located, found("Warsaw")));
    CHECK(located.m_located_name.view() == "Warsaw, Masovia");
}

TEST_CASE("match_report empties a report in other units than the settings")
{
    // units saved during the lookup: the forecast comes in mph, so km/h values and their label must go
    WeatherReport report;
    report.m_place.append("Warsaw");
    report.m_has_values = true;
    report.m_wind_speed = 12;
    report.m_revision = 7;
    WeatherSettings settings = found("Warsaw");
    settings.m_units = MeasurementSystem::imperial;
    REQUIRE(match_report(report, settings));
    CHECK(report.m_units == MeasurementSystem::imperial);
    CHECK(report.m_place.view() == "Warsaw");
    CHECK_FALSE(report.m_has_values);
    CHECK(report.m_wind_speed == 0);
    CHECK(report.m_revision == 7);
}

TEST_CASE("match_report keeps a report that belongs to the settings")
{
    WeatherReport report;
    report.m_place.append("Warsaw");
    report.m_has_values = true;
    report.m_wind_speed = 12;
    CHECK_FALSE(match_report(report, found("Warsaw")));
    CHECK(report.m_has_values);
    CHECK(report.m_wind_speed == 12);

    CHECK(match_report(report, found("Krakow")));
    CHECK(report.m_place.view() == "Krakow");
    CHECK_FALSE(report.m_has_values);
}

TEST_CASE("merged_settings keeps what isn't given from the stored record")
{
    WeatherSettings stored = found("Warsaw");
    stored.m_units = MeasurementSystem::imperial;

    const WeatherSettings units_only = merged_settings(stored, std::nullopt, "metric");
    CHECK(units_only.m_place.view() == "Warsaw");
    CHECK(units_only.m_units == MeasurementSystem::metric);
    CHECK(units_only.m_located);  // same place: its coordinates stay
    CHECK(units_only.m_latitude_micro_degrees == warsaw_latitude);
    CHECK(units_only.m_located_name.view() == "Warsaw, Poland");

    const WeatherSettings place_only = merged_settings(stored, "Krakow", std::nullopt);
    CHECK(place_only.m_place.view() == "Krakow");
    CHECK(place_only.m_units == MeasurementSystem::imperial);
    CHECK_FALSE(place_only.m_located);  // another place: looked up again
    CHECK(place_only.m_latitude_micro_degrees == 0);
    CHECK(place_only.m_located_name.view().empty());
}

}  // namespace
}  // namespace avela
