#include "avela_radio/model/carry_out.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>

namespace avela {
namespace {

TEST_CASE("carrying out a request changes what it gives and keeps the rest")
{
    RadioModel model;
    model.m_playing = false;
    model.m_volume = 40;
    carry_out(PlayerRequest{.m_playing = true}, model);
    CHECK(model.m_playing);
    CHECK(model.m_volume == 40);
    CHECK(model.m_station_index == 0);

    carry_out(PlayerRequest{.m_volume = 55}, model);
    CHECK(model.m_volume == 55);
    CHECK(model.m_playing);
}

TEST_CASE("changes add up: requests queued before they are carried out lose nothing")
{
    RadioModel model;
    model.m_volume = 40;
    model.m_playing = true;
    for (int step = 0; step < 3; ++step) {
        carry_out(PlayerRequest{.m_volume_change = 2}, model);
    }
    CHECK(model.m_volume == 46);
    carry_out(PlayerRequest{.m_toggle_playing = true}, model);
    CHECK_FALSE(model.m_playing);
    carry_out(PlayerRequest{.m_toggle_playing = true}, model);
    CHECK(model.m_playing);
}

TEST_CASE("the volume is clamped")
{
    RadioModel model;
    carry_out(PlayerRequest{.m_volume = RadioModel::volume_max + 7}, model);
    CHECK(model.m_volume == RadioModel::volume_max);
    carry_out(PlayerRequest{.m_volume_change = -(RadioModel::volume_max + 3)}, model);
    CHECK(model.m_volume == RadioModel::volume_min);
}

TEST_CASE("the station is chosen by its slot; a slot without one keeps the station")
{
    RadioModel model;
    const uint8_t third = model.m_stations.slot(2);
    carry_out(PlayerRequest{.m_station_slot = third}, model);
    CHECK(model.m_station_index == 2);
    carry_out(PlayerRequest{.m_station_slot = StationRecord::slot_count}, model);  // no default station there
    CHECK(model.m_station_index == 2);
}

}  // namespace
}  // namespace avela
