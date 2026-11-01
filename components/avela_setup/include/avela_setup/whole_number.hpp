// A field's value as a whole number ("42"), for the sections' apply(): nothing else around it, no sign but '-'.
#pragma once

#include <charconv>
#include <optional>
#include <string_view>
#include <system_error>

namespace avela {

inline std::optional<int> whole_number(std::string_view text)
{
    int number = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), number);
    if (error != std::errc{} || end != text.data() + text.size()) {
        return std::nullopt;
    }
    return number;
}

}  // namespace avela
