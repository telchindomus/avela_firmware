// The stations section (setup page and serial console): up to 16 stations, each a name, a genre and the address of
// its MP3 stream, in numbered slots (fields "3.name", "3.genre", "3.url"; station_fields.hpp). An empty address
// removes the station; the last station can't be removed. Saving stores StationRecords; RadioStorage sees the new
// revision and reloads the list. The page shows one form per station (the used slots and one free), since a form
// holds at most FormFields::capacity bytes. Runs on the web server's or the console's task: reads and writes only
// Settings.
#pragma once

#include <array>
#include <string_view>

#include "avela_radio/setup/station_fields.hpp"
#include "avela_setup/apply_result.hpp"
#include "avela_setup/setup_field.hpp"
#include "avela_setup/setup_section_concept.hpp"

namespace avela {

class StationsSection {
public:
    static constexpr std::string_view id = "stations";
    static constexpr std::string_view title = "Stations";
    static constexpr bool writes_own_forms = true;  // a form per station
    static constexpr std::array fields = [] {
        constexpr std::array<std::string_view, station_fields::part_names.size()> descriptions{
            "the station's name, up to 32 characters",
            "its genre, up to 32 characters (may be empty)",
            "its MP3 stream, http://... or https://... (empty: no station here)",
        };
        std::array<SetupField, station_fields::count> all{};
        for (std::size_t field = 0; field < all.size(); ++field) {
            all.at(field) = SetupField{.m_name = station_fields::names.at(field).view(),
                                       .m_description = descriptions.at(field % descriptions.size()),
                                       .m_secret = false};
        }
        return all;
    }();

    static void write_form(HtmlWriter& writer);
    static ApplyResult apply(const FormFields& submitted);
    static void write_values(ValueWriter& values);
};

static_assert(SetupSectionConcept<StationsSection>);

}  // namespace avela
