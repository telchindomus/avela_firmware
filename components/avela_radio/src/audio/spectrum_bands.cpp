#include "avela_radio/audio/spectrum_bands.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "avela_radio/model/radio_model.hpp"

namespace avela {
namespace {

constexpr float decibels_per_power_decade = 10.0f;

// The band's edge: lowest_frequency for edge 0, highest_frequency for edge count, evenly spaced in between on a log
// scale.
float band_edge(std::size_t edge, std::size_t count)
{
    const float ratio = SpectrumScale::highest_frequency / SpectrumScale::lowest_frequency;
    return SpectrumScale::lowest_frequency * std::pow(ratio, static_cast<float>(edge) / static_cast<float>(count));
}

uint8_t level_of(float power)
{
    if (power <= 0.0f) {
        return 0;
    }
    const float decibels = decibels_per_power_decade * std::log10(power);
    const float share = std::clamp(1.0f - (decibels / SpectrumScale::floor_decibels), 0.0f, 1.0f);
    return static_cast<uint8_t>(std::lround(share * RadioModel::spectrum_level_max));
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void spectrum_levels(std::span<const float> bin_power, uint32_t sample_rate, std::span<uint8_t> levels)
{
    if (bin_power.empty() || sample_rate == 0) {
        std::ranges::fill(levels, uint8_t{0});
        return;
    }
    const float bin_width = static_cast<float>(sample_rate) / static_cast<float>(2 * bin_power.size());
    const float nyquist_frequency = static_cast<float>(sample_rate) / 2.0f;
    const std::size_t last_bin = bin_power.size() - 1;
    // the bin whose middle is nearest to frequency
    const auto bin_at = [&](float frequency) {
        return std::min(static_cast<std::size_t>(std::lround(frequency / bin_width)), last_bin);
    };
    for (std::size_t band = 0; band < levels.size(); ++band) {
        const float low = band_edge(band, levels.size());
        const float high = band_edge(band + 1, levels.size());
        if (low >= nyquist_frequency) {
            levels[band] = 0;  // above what the stream can hold (low sample rates)
            continue;
        }
        // the bins with their middle in [low, high); none if the band is narrower than a bin
        const auto first = static_cast<std::size_t>(std::ceil(low / bin_width));
        const std::size_t end = std::min(static_cast<std::size_t>(std::ceil(high / bin_width)), last_bin + 1);
        float power = 0.0f;
        if (first < end) {
            power = *std::max_element(bin_power.begin() + static_cast<std::ptrdiff_t>(first),
                                      bin_power.begin() + static_cast<std::ptrdiff_t>(end));
        } else {
            power = bin_power[bin_at(std::sqrt(low * high))];
        }
        levels[band] = level_of(power);
    }
}

}  // namespace avela
