// A Stations field name as text in static storage ("12.url").
#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace avela::station_fields {

struct Name {
    std::array<char, sizeof("16.genre")> m_text{};
    std::size_t m_length = 0;

    [[nodiscard]] constexpr std::string_view view() const { return {m_text.data(), m_length}; }
};

}  // namespace avela::station_fields
