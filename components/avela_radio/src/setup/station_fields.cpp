#include "avela_radio/setup/station_fields.hpp"

#include <charconv>

namespace avela::station_fields {

std::optional<Field> parse(std::string_view name)
{
    const std::size_t dot = name.find('.');
    if (dot == std::string_view::npos) {
        return std::nullopt;
    }
    unsigned slot = 0;
    const std::string_view number = name.substr(0, dot);
    const auto [end, error] = std::from_chars(number.data(), number.data() + number.size(), slot);
    if (error != std::errc{} || end != number.data() + number.size() || number.starts_with('0') || slot < 1 ||
        slot > StationRecord::slot_count) {
        return std::nullopt;
    }
    const std::string_view part = name.substr(dot + 1);
    for (std::size_t index = 0; index < part_names.size(); ++index) {
        if (part == part_names.at(index)) {
            return Field{.m_slot = static_cast<uint8_t>(slot), .m_part = static_cast<Part>(index)};
        }
    }
    return std::nullopt;
}

}  // namespace avela::station_fields
