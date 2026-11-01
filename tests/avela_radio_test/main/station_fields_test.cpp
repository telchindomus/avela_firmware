#include "avela_radio/setup/station_fields.hpp"

#include <catch2/catch_test_macros.hpp>

namespace avela {
namespace {

using station_fields::Field;
using station_fields::Part;

TEST_CASE("the field names run through the slots in order")
{
    CHECK(station_fields::names.front().view() == "1.name");
    CHECK(station_fields::names.at(4).view() == "2.genre");
    CHECK(station_fields::names.back().view() == "16.url");
    CHECK(station_fields::name(12, Part::url) == "12.url");
}

TEST_CASE("a field name gives its slot and part")
{
    CHECK(station_fields::parse("12.genre") == Field{.m_slot = 12, .m_part = Part::genre});
    CHECK(station_fields::parse("1.url") == Field{.m_slot = 1, .m_part = Part::url});
}

TEST_CASE("other names are no station field")
{
    CHECK_FALSE(station_fields::parse("section").has_value());
    CHECK_FALSE(station_fields::parse("0.name").has_value());
    CHECK_FALSE(station_fields::parse("17.name").has_value());
    CHECK_FALSE(station_fields::parse("01.name").has_value());
    CHECK_FALSE(station_fields::parse("3.title").has_value());
    CHECK_FALSE(station_fields::parse(".name").has_value());
    CHECK_FALSE(station_fields::parse("3x.name").has_value());
}

TEST_CASE("every name parses back to its own slot and part")
{
    for (std::size_t slot = 1; slot <= StationRecord::slot_count; ++slot) {
        for (const Part part : {Part::name, Part::genre, Part::url}) {
            CHECK(station_fields::parse(station_fields::name(slot, part)) ==
                  Field{.m_slot = static_cast<uint8_t>(slot), .m_part = part});
        }
    }
}

}  // namespace
}  // namespace avela
