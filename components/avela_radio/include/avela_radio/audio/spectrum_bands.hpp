// From an FFT's power per frequency bin to the RadioModel's spectrum: spectrum_band_count bands spaced evenly on a
// logarithmic scale (as the ear hears pitch) from lowest_frequency to highest_frequency, each band's level the
// strongest bin in it, in decibels from the floor (level 0) to full scale (spectrum_level_max). A band narrower than a
// bin takes the bin under its middle; a band above half the sample rate stays at 0. Platform-free (the FFT itself is
// SpectrumAnalyzer's).
#pragma once

#include <cstdint>
#include <span>

namespace avela {

struct SpectrumScale {
    static constexpr float lowest_frequency = 50.0f;      // Hz, the lowest band's lower edge
    static constexpr float highest_frequency = 16000.0f;  // Hz, the highest band's upper edge
    static constexpr float floor_decibels = -60.0f;       // this quiet or quieter is level 0
};

// bin_power: bins 0..N/2-1 of an N-point FFT of a stream at sample_rate, scaled so that a full-scale sine is 1.0.
// levels: one per band, low -> high.
void spectrum_levels(std::span<const float> bin_power, uint32_t sample_rate, std::span<uint8_t> levels);

}  // namespace avela
