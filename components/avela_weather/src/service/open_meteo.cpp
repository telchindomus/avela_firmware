#include "avela_weather/service/open_meteo.hpp"

#include <array>
#include <charconv>
#include <cstdint>

namespace avela::open_meteo {
namespace {

bool is_space(char character)
{
    return character == ' ' || character == '\n' || character == '\r' || character == '\t';
}

std::size_t skip_spaces(std::string_view text, std::size_t position)
{
    while (position < text.size() && is_space(text[position])) {
        ++position;
    }
    return position;
}

// The end of the string that starts at `position` (a '"'): the position after its closing quote.
std::size_t string_end(std::string_view text, std::size_t position)
{
    for (++position; position < text.size(); ++position) {
        if (text[position] == '\\') {
            ++position;  // the escaped character
        } else if (text[position] == '"') {
            return position + 1;
        }
    }
    return text.size();
}

// The end of the value that starts at `position`: after its closing quote or bracket, or where a number ends.
std::size_t value_end(std::string_view text, std::size_t position)
{
    if (position >= text.size()) {
        return text.size();
    }
    if (text[position] == '"') {
        return string_end(text, position);
    }
    if (text[position] == '{' || text[position] == '[') {
        int depth = 0;
        while (position < text.size()) {
            const char character = text[position];
            if (character == '"') {
                position = string_end(text, position);
                continue;
            }
            if (character == '{' || character == '[') {
                ++depth;
            } else if (character == '}' || character == ']') {
                --depth;
                if (depth == 0) {
                    return position + 1;
                }
            }
            ++position;
        }
        return text.size();
    }
    while (position < text.size() && text[position] != ',' && text[position] != '}' && text[position] != ']' &&
           !is_space(text[position])) {
        ++position;
    }
    return position;
}

// A whole number that is all of `text`.
std::optional<int> whole_number(std::string_view text)
{
    int result = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), result);
    if (error != std::errc{} || end != text.data() + text.size() || text.empty()) {
        return std::nullopt;
    }
    return result;
}

}  // namespace

//--------------------------------------------------------------------------------------------------

std::string_view value_of(std::string_view object, std::string_view key)
{
    std::size_t position = skip_spaces(object, 0);
    if (position >= object.size() || object[position] != '{') {
        return {};
    }
    ++position;
    while (true) {
        position = skip_spaces(object, position);
        if (position >= object.size() || object[position] != '"') {
            return {};  // '}' (end of the object) or not JSON
        }
        const std::size_t key_end = string_end(object, position);
        const std::string_view found = object.substr(position + 1, key_end - position - 2);
        position = skip_spaces(object, key_end);
        if (position >= object.size() || object[position] != ':') {
            return {};
        }
        position = skip_spaces(object, position + 1);
        const std::size_t end = value_end(object, position);
        if (found == key) {
            return object.substr(position, end - position);
        }
        position = skip_spaces(object, end);
        if (position >= object.size() || object[position] != ',') {
            return {};
        }
        ++position;
    }
}

//--------------------------------------------------------------------------------------------------

std::optional<double> number(std::string_view value)
{
    double result = 0.0;
    const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), result);
    if (error != std::errc{} || end != value.data() + value.size() || value.empty()) {
        return std::nullopt;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------

std::optional<std::string_view> string(std::string_view value)
{
    constexpr std::size_t quotes = 2;
    if (value.size() < quotes || value.front() != '"' || value.back() != '"') {
        return std::nullopt;
    }
    return value.substr(1, value.size() - quotes);
}

//--------------------------------------------------------------------------------------------------

std::size_t elements(std::string_view array, std::span<std::string_view> elements)
{
    std::size_t position = skip_spaces(array, 0);
    if (position >= array.size() || array[position] != '[') {
        return 0;
    }
    std::size_t count = 0;
    position = skip_spaces(array, position + 1);
    while (count < elements.size() && position < array.size() && array[position] != ']') {
        const std::size_t end = value_end(array, position);
        elements[count] = array.substr(position, end - position);
        ++count;
        position = skip_spaces(array, end);
        if (position < array.size() && array[position] == ',') {
            position = skip_spaces(array, position + 1);
        }
    }
    return count;
}

//--------------------------------------------------------------------------------------------------

WeatherCondition condition_of(int weather_code)
{
    // WMO 4677 as Open-Meteo documents it: 0 clear, 1 mainly clear, 2 partly cloudy, 3 overcast, 45/48 fog,
    // 51-57 drizzle, 61-67 rain, 71-77 snow, 80-82 rain showers, 85/86 snow showers, 95-99 thunderstorm
    constexpr int mainly_clear = 1;
    constexpr int partly_cloudy = 2;
    constexpr int overcast = 3;
    constexpr int fog_last = 48;
    constexpr int drizzle_last = 57;
    constexpr int rain_last = 67;
    constexpr int snow_last = 77;
    constexpr int showers_last = 82;
    constexpr int snow_showers_last = 86;
    if (weather_code <= mainly_clear) {
        return WeatherCondition::clear;
    }
    if (weather_code == partly_cloudy) {
        return WeatherCondition::partly_cloudy;
    }
    if (weather_code == overcast) {
        return WeatherCondition::cloudy;
    }
    if (weather_code <= fog_last) {
        return WeatherCondition::fog;
    }
    if (weather_code <= drizzle_last) {
        return WeatherCondition::drizzle;
    }
    if (weather_code <= rain_last) {
        return WeatherCondition::rain;
    }
    if (weather_code <= snow_last) {
        return WeatherCondition::snow;
    }
    if (weather_code <= showers_last) {
        return WeatherCondition::rain;
    }
    if (weather_code <= snow_showers_last) {
        return WeatherCondition::snow;
    }
    return WeatherCondition::thunderstorm;
}

//--------------------------------------------------------------------------------------------------

std::optional<int> weekday_of(std::string_view date)
{
    // YYYY-MM-DD
    constexpr std::size_t iso_date_length = 10;
    constexpr std::size_t year_digits = 4;
    constexpr std::size_t month_at = 5;
    constexpr std::size_t day_at = 8;
    constexpr std::size_t two_digits = 2;
    if (date.size() != iso_date_length || date[month_at - 1] != '-' || date[day_at - 1] != '-') {
        return std::nullopt;
    }
    const std::optional<int> read_year = whole_number(date.substr(0, year_digits));
    const std::optional<int> month = whole_number(date.substr(month_at, two_digits));
    const std::optional<int> day = whole_number(date.substr(day_at, two_digits));
    if (!read_year || !month || !day) {
        return std::nullopt;
    }
    int year = *read_year;
    // Sakamoto's method: the weekday of a Gregorian date, 0 = Sunday
    constexpr std::array<int, 12> month_offsets{0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    constexpr int months = 12;
    constexpr int days_per_week = 7;
    constexpr int leap_every = 4;
    constexpr int century = 100;
    constexpr int leap_century_every = 400;
    if (*month < 1 || *month > months) {
        return std::nullopt;
    }
    constexpr int march = 3;
    if (*month < march) {
        --year;
    }
    return (year + year / leap_every - year / century + year / leap_century_every +
            month_offsets.at(static_cast<std::size_t>(*month - 1)) + *day) %
           days_per_week;
}

}  // namespace avela::open_meteo
