#include "avela_weather/service/open_meteo.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <string_view>

namespace avela::open_meteo {
namespace {

using namespace std::string_view_literals;

TEST_CASE("value_of finds a key's raw value in an object")
{
    constexpr std::string_view object = R"({ "latitude": 52.23, "name": "Warsaw", "daily": {"time": [1, 2]} })";
    CHECK(value_of(object, "latitude") == "52.23");
    CHECK(value_of(object, "name") == R"("Warsaw")");
    CHECK(value_of(object, "daily") == R"({"time": [1, 2]})");
}

TEST_CASE("value_of looks only at the object's own keys")
{
    constexpr std::string_view object = R"({"daily": {"time": [1]}, "other": 1})";
    CHECK(value_of(object, "time").empty());
    CHECK(value_of(object, "missing").empty());
}

TEST_CASE("value_of skips strings with escaped quotes and brackets")
{
    constexpr std::string_view object = R"({"text": "a \"[quoted]\" {word}", "after": 7})";
    CHECK(value_of(object, "after") == "7");
}

TEST_CASE("value_of gives nothing for what isn't an object")
{
    CHECK(value_of("[1, 2]", "a").empty());
    CHECK(value_of("", "a").empty());
    CHECK(value_of(R"({"a" 1})", "a").empty());
}

TEST_CASE("number reads a whole value only")
{
    CHECK(number("-3.5") == -3.5);
    CHECK(number("12") == 12.0);
    CHECK_FALSE(number("12abc").has_value());
    CHECK_FALSE(number("").has_value());
    CHECK_FALSE(number(R"("12")").has_value());
}

TEST_CASE("string gives the text between the quotes")
{
    CHECK(string(R"("Warsaw")") == "Warsaw"sv);
    CHECK(string(R"("")") == ""sv);
    CHECK_FALSE(string("12").has_value());
    CHECK_FALSE(string(R"(")").has_value());
}

TEST_CASE("elements splits an array into raw values")
{
    std::array<std::string_view, 4> found{};
    REQUIRE(elements(R"([ 1.5, "two", [3], {"four": 4} ])", found) == 4);
    CHECK(found[0] == "1.5");
    CHECK(found[1] == R"("two")");
    CHECK(found[2] == "[3]");
    CHECK(found[3] == R"({"four": 4})");
}

TEST_CASE("elements stops when the span is full and handles empty arrays")
{
    std::array<std::string_view, 2> found{};
    CHECK(elements("[1, 2, 3]", found) == 2);
    CHECK(elements("[]", found) == 0);
    CHECK(elements("1", found) == 0);
}

TEST_CASE("condition_of groups the WMO codes")
{
    CHECK(condition_of(0) == WeatherCondition::clear);
    CHECK(condition_of(1) == WeatherCondition::clear);
    CHECK(condition_of(2) == WeatherCondition::partly_cloudy);
    CHECK(condition_of(3) == WeatherCondition::cloudy);
    CHECK(condition_of(45) == WeatherCondition::fog);
    CHECK(condition_of(53) == WeatherCondition::drizzle);
    CHECK(condition_of(63) == WeatherCondition::rain);
    CHECK(condition_of(75) == WeatherCondition::snow);
    CHECK(condition_of(81) == WeatherCondition::rain);
    CHECK(condition_of(86) == WeatherCondition::snow);
    CHECK(condition_of(95) == WeatherCondition::thunderstorm);
}

TEST_CASE("weekday_of reads ISO dates (0 = Sunday)")
{
    CHECK(weekday_of("2026-10-24") == 6);  // a Saturday
    CHECK(weekday_of("2024-02-29") == 4);  // a leap day, Thursday
    CHECK(weekday_of("2000-01-01") == 6);
    CHECK_FALSE(weekday_of("2026-13-01").has_value());
    CHECK_FALSE(weekday_of("2026/10/24").has_value());
    CHECK_FALSE(weekday_of("24-10-2026").has_value());
}

}  // namespace
}  // namespace avela::open_meteo
