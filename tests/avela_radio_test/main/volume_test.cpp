#include "avela_radio/audio/volume.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "avela_radio/model/radio_model.hpp"

namespace avela {
namespace {

TEST_CASE("the top of the scale is the stream as it is, the bottom silence")
{
    CHECK(volume_gain(RadioModel::volume_max) == unity_gain);
    CHECK(volume_gain(RadioModel::volume_min) == 0);
    CHECK(volume_gain(RadioModel::volume_max + 5) == unity_gain);
    CHECK(volume_gain(RadioModel::volume_min - 5) == 0);
}

TEST_CASE("the gain follows the position")
{
    CHECK(volume_gain(99) == 32440);  // 32768 * 0.99, rounded
    CHECK(volume_gain(50) == 16384);  // half the amplitude: -6 dB in the middle
    CHECK(volume_gain(25) == 8192);   // -12 dB
    CHECK(volume_gain(1) == 328);     // 32768 * 0.01, rounded: -40 dB
}

TEST_CASE("louder is never quieter")
{
    for (int volume = RadioModel::volume_min; volume < RadioModel::volume_max; ++volume) {
        CAPTURE(volume);
        CHECK(volume_gain(volume) < volume_gain(volume + 1));
    }
}

TEST_CASE("apply_gain scales every sample")
{
    std::array<int16_t, 4> samples{1000, -1000, 32767, -32768};
    apply_gain(samples, unity_gain);
    CHECK(samples == std::array<int16_t, 4>{1000, -1000, 32767, -32768});
    apply_gain(samples, unity_gain / 2);
    CHECK(samples == std::array<int16_t, 4>{500, -500, 16383, -16384});
    apply_gain(samples, 0);
    CHECK(samples == std::array<int16_t, 4>{0, 0, 0, 0});
}

}  // namespace
}  // namespace avela
