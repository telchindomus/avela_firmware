#include "avela_radio/model/station_list.hpp"
#include "avela_radio/model/default_station.hpp"

namespace avela {
namespace {

constexpr std::size_t monogram_words = 3;
constexpr std::size_t one_word_letters = 2;
constexpr unsigned utf8_continuation_mask = 0xC0U;  // a continuation byte is 10xxxxxx
constexpr unsigned utf8_continuation = 0x80U;

// The bytes of the UTF-8 character that starts text.
std::string_view first_character(std::string_view text)
{
    std::size_t length = 1;
    while (length < text.size() &&
           (static_cast<unsigned char>(text[length]) & utf8_continuation_mask) == utf8_continuation) {
        ++length;  // a continuation byte
    }
    return text.substr(0, length);
}

void append_upper(Monogram& monogram, std::string_view character)
{
    if (character.size() == 1 && character[0] >= 'a' && character[0] <= 'z') {
        const char upper = static_cast<char>(character[0] - 'a' + 'A');
        monogram.append(std::string_view{&upper, 1});
    } else {
        monogram.append(character);
    }
}

}  // namespace

//--------------------------------------------------------------------------------------------------

Monogram station_monogram(std::string_view name)
{
    std::array<std::string_view, monogram_words> words{};
    std::size_t count = 0;
    while (!name.empty() && count < words.size()) {
        const std::size_t start = name.find_first_not_of(' ');
        if (start == std::string_view::npos) {
            break;
        }
        name.remove_prefix(start);
        const std::size_t end = std::min(name.find(' '), name.size());
        words.at(count++) = name.substr(0, end);
        name.remove_prefix(end);
    }
    Monogram monogram;
    if (count == 1) {
        std::string_view word = words[0];
        for (std::size_t letter = 0; letter < one_word_letters && !word.empty(); ++letter) {
            const std::string_view character = first_character(word);
            append_upper(monogram, character);
            word.remove_prefix(character.size());
        }
        return monogram;
    }
    for (std::size_t word = 0; word < count; ++word) {
        append_upper(monogram, first_character(words.at(word)));
    }
    return monogram;
}

//--------------------------------------------------------------------------------------------------

StationList::StationList()
{
    uint8_t slot = 1;
    for (const DefaultStation& station : default_stations) {
        add(slot++, station.m_name, station.m_genre);
    }
}

//--------------------------------------------------------------------------------------------------

void StationList::clear()
{
    m_count_ = 0;
}

//--------------------------------------------------------------------------------------------------

bool StationList::add(uint8_t slot, std::string_view name, std::string_view genre)
{
    if (m_count_ == capacity) {
        return false;
    }
    Entry& entry = m_entries_.at(m_count_);
    entry.m_name.clear().append(name);
    entry.m_genre.clear().append(genre);
    entry.m_monogram = station_monogram(name);
    entry.m_slot = slot;
    m_stations_.at(m_count_) = Station{
        .m_name = entry.m_name.c_str(), .m_monogram = entry.m_monogram.c_str(), .m_genre = entry.m_genre.c_str()};
    ++m_count_;
    return true;
}

//--------------------------------------------------------------------------------------------------

std::optional<std::size_t> StationList::index_of(uint8_t slot) const
{
    for (std::size_t index = 0; index < m_count_; ++index) {
        if (m_entries_.at(index).m_slot == slot) {
            return index;
        }
    }
    return std::nullopt;
}

}  // namespace avela
