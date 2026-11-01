#include "avela_radio/model/default_station.hpp"
#include "avela_radio/model/station_list.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string_view>

namespace avela {
namespace {

TEST_CASE("a monogram takes the first letters of up to three words")
{
    CHECK(station_monogram("Radio Swiss Jazz").view() == "RSJ");
    CHECK(station_monogram("Groove Salad").view() == "GS");
    CHECK(station_monogram("radio  paradise").view() == "RP");
    CHECK(station_monogram("The Big Old Band Radio").view() == "TBO");
}

TEST_CASE("a one-word name gives its first two letters")
{
    CHECK(station_monogram("Kexp").view() == "KE");
    CHECK(station_monogram("X").view() == "X");
    CHECK(station_monogram("").view().empty());
}

TEST_CASE("a monogram keeps whole UTF-8 characters")
{
    CHECK(station_monogram("Österreich 1").view() == "Ö1");
    CHECK(station_monogram("Ñu").view() == "ÑU");
}

TEST_CASE("the list starts with the default stations in slots 1..")
{
    const StationList list;
    REQUIRE(list.size() == default_stations.size());
    CHECK(std::string_view{list.stations()[0].m_name} == default_stations[0].m_name);
    CHECK(std::string_view{list.stations()[3].m_monogram} == "RSJ");
    CHECK(list.slot(0) == 1);
    CHECK(list.index_of(2) == 1);
}

TEST_CASE("stations keep their slots, gaps included")
{
    StationList list;
    list.clear();
    CHECK(list.add(3, "FIP Jazz", "Jazz"));
    CHECK(list.add(9, "Groove Salad", "Ambient"));
    REQUIRE(list.size() == 2);
    CHECK(list.slot(1) == 9);
    CHECK(list.index_of(9) == 1);
    CHECK_FALSE(list.index_of(1).has_value());
    CHECK(std::string_view{list.stations()[1].m_genre} == "Ambient");
}

TEST_CASE("the list holds at most capacity stations")
{
    StationList list;
    list.clear();
    for (std::size_t slot = 1; slot <= StationList::capacity; ++slot) {
        CHECK(list.add(static_cast<uint8_t>(slot), "Station", ""));
    }
    CHECK_FALSE(list.add(1, "One too many", ""));
}

}  // namespace
}  // namespace avela
