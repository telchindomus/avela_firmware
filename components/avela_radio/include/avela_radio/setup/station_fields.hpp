// The Stations section's field names: "<slot>.name", "<slot>.genre", "<slot>.url" for slots 1..16 ("12.url"), the same
// on the setup page, the console and in settings files. Platform-free.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

#include "avela_radio/model/station_record.hpp"
#include "avela_radio/setup/station_fields/field.hpp"
#include "avela_radio/setup/station_fields/name.hpp"
#include "avela_radio/setup/station_fields/part.hpp"

namespace avela::station_fields {

inline constexpr std::array part_names{std::string_view{"name"}, std::string_view{"genre"}, std::string_view{"url"}};
inline constexpr std::size_t count = StationRecord::slot_count * part_names.size();

// "12.url" -> {12, url}; nothing for any other name.
std::optional<Field> parse(std::string_view name);

// Every field name in slot order: "1.name", "1.genre", "1.url", "2.name" ...
consteval std::array<Name, count> make_names()
{
    std::array<Name, count> names{};
    for (std::size_t field = 0; field < count; ++field) {
        const std::size_t slot = (field / part_names.size()) + 1;
        const std::string_view part = part_names.at(field % part_names.size());
        Name& name = names.at(field);
        constexpr std::size_t decimal = 10;
        if (slot >= decimal) {
            name.m_text.at(name.m_length++) = static_cast<char>('0' + (slot / decimal));
        }
        name.m_text.at(name.m_length++) = static_cast<char>('0' + (slot % decimal));
        name.m_text.at(name.m_length++) = '.';
        for (const char character : part) {
            name.m_text.at(name.m_length++) = character;
        }
    }
    return names;
}

inline constexpr std::array<Name, count> names = make_names();

// The name of a slot's field ("12.url").
constexpr std::string_view name(std::size_t slot, Part part)
{
    return names.at(((slot - 1) * part_names.size()) + static_cast<std::size_t>(part)).view();
}

}  // namespace avela::station_fields
