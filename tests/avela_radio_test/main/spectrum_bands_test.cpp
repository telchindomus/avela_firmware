#include "avela_radio/audio/spectrum_bands.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>

#include "avela_radio/model/radio_model.hpp"

namespace avela {
namespace {

constexpr std::size_t bin_count = 256;  // a 512-point FFT
using Bins = std::array<float, bin_count>;
using Levels = std::array<uint8_t, RadioModel::spectrum_band_count>;

Levels levels_of(const Bins& bins, uint32_t sample_rate)
{
    Levels levels{};
    spectrum_levels(bins, sample_rate, levels);
    return levels;
}

TEST_CASE("silence is level 0 everywhere")
{
    CHECK(levels_of(Bins{}, 44'100) == Levels{});
}

TEST_CASE("a full-scale 1 kHz sine fills its band only")
{
    Bins bins{};
    bins[12] = 1.0f;  // 44.1 kHz / 512 = 86 Hz per bin: bin 12 is 1034 Hz
    const Levels levels = levels_of(bins, 44'100);
    // the bands' edges are 50 Hz * 320^(band / 32): band 16 is 894..1070 Hz
    for (std::size_t band = 0; band < levels.size(); ++band) {
        CAPTURE(band);
        CHECK(levels[band] == (band == 16 ? RadioModel::spectrum_level_max : 0));
    }
}

TEST_CASE("levels run in decibels from the floor to full scale")
{
    Bins bins{};
    bins[12] = 1.0e-2f;  // -20 dB: two thirds of the way up from -60 dB
    CHECK(levels_of(bins, 44'100)[16] == 170);
    bins[12] = 1.0e-6f;  // -60 dB: the floor
    CHECK(levels_of(bins, 44'100)[16] == 0);
    bins[12] = 1.0e-9f;  // below the floor
    CHECK(levels_of(bins, 44'100)[16] == 0);
}

TEST_CASE("a band narrower than a bin takes the bin under its middle")
{
    Bins bins{};
    bins[1] = 1.0f;  // 86 Hz: under the lowest bands' middles (50..60 Hz is band 0)
    CHECK(levels_of(bins, 44'100)[0] == RadioModel::spectrum_level_max);
}

TEST_CASE("bands above half the sample rate stay at 0")
{
    Bins bins{};
    bins.back() = 1.0f;  // at 22.05 kHz: 11 kHz, the highest the stream holds
    const Levels levels = levels_of(bins, 22'050);
    CHECK(levels[29] == RadioModel::spectrum_level_max);  // 9.3..11.2 kHz
    CHECK(levels[30] == 0);                               // from 11.2 kHz: above 11 kHz
    CHECK(levels[31] == 0);
}

}  // namespace
}  // namespace avela
