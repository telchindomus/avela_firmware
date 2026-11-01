#include "avela_radio/audio/volume.hpp"

#include <algorithm>

#include "avela_radio/model/radio_model.hpp"

namespace avela {
namespace {

constexpr int gain_shift = 15;  // unity_gain = 1 << gain_shift

}  // namespace

//--------------------------------------------------------------------------------------------------

int32_t volume_gain(int volume)
{
    const int64_t position =
        std::clamp(volume, RadioModel::volume_min, RadioModel::volume_max) - RadioModel::volume_min;
    const int64_t full_scale = RadioModel::volume_max - RadioModel::volume_min;
    // unity_gain * position / full_scale, rounded
    return static_cast<int32_t>((unity_gain * position + full_scale / 2) / full_scale);
}

//--------------------------------------------------------------------------------------------------

void apply_gain(std::span<int16_t> samples, int32_t gain)
{
    for (int16_t& sample : samples) {
        sample = static_cast<int16_t>((int32_t{sample} * gain) >> gain_shift);
    }
}

}  // namespace avela
