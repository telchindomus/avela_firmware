// The stations as the screens list them: name, genre and monogram of every used station slot, in slot order (the
// addresses stay in the StationRecords). Starts with the default stations, until RadioStorage loads the saved ones;
// never empty (the Stations section keeps at least one station). Platform-free.
// The Station entries point into this object's own texts: not copyable.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

#include "avela_radio/model/station.hpp"
#include "avela_radio/model/station_record.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela {

// Up to three characters (UTF-8, so up to 4 bytes each).
using Monogram = FixedString<(3 * 4) + 1>;

// The first letters of the name's first three words ("Radio Swiss Jazz" -> "RSJ"), or the first two letters of a
// one-word name ("Kexp" -> "KE"); ASCII letters upper-cased.
Monogram station_monogram(std::string_view name);

class StationList {
public:
    static constexpr std::size_t capacity = StationRecord::slot_count;

    // The default stations, in slots 1..
    StationList();
    StationList(const StationList&) = delete;
    StationList& operator=(const StationList&) = delete;
    StationList(StationList&&) = delete;
    StationList& operator=(StationList&&) = delete;
    ~StationList() = default;

    void clear();
    // Adds the station in `slot` (1..capacity) after the ones added so far; false if the list is full.
    bool add(uint8_t slot, std::string_view name, std::string_view genre);

    [[nodiscard]] std::span<const Station> stations() const { return std::span{m_stations_}.first(m_count_); }
    [[nodiscard]] std::size_t size() const { return m_count_; }
    // The slot of the station at `index` (< size()).
    [[nodiscard]] uint8_t slot(std::size_t index) const { return m_entries_.at(index).m_slot; }
    // The index of the station in `slot`, if that slot holds one.
    [[nodiscard]] std::optional<std::size_t> index_of(uint8_t slot) const;

private:
    struct Entry {
        FixedString<StationRecord::name_capacity> m_name;
        FixedString<StationRecord::genre_capacity> m_genre;
        Monogram m_monogram;
        uint8_t m_slot = 0;
    };

    std::array<Entry, capacity> m_entries_{};
    std::array<Station, capacity> m_stations_{};
    std::size_t m_count_ = 0;
};

}  // namespace avela
