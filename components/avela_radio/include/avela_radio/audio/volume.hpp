// The volume (RadioModel: volume_min..volume_max) as a gain on the samples: the top of the scale is the stream as it
// is, the bottom silence, and in between the gain follows the position (the middle is half the amplitude, -6 dB; a
// quarter is -12 dB). On the board, 0.5 dB per step (-25 dB in the middle) and the square of the position (-24 dB
// at a quarter) were both too quiet. Platform-free.
#pragma once

#include <cstdint>
#include <span>

namespace avela {

// The gain as a fraction of unity_gain (fixed point, 15 bits): samples are multiplied by it.
inline constexpr int32_t unity_gain = int32_t{1} << 15;

// unity_gain at volume_max, unity_gain * (position on the scale) below, 0 at volume_min.
int32_t volume_gain(int volume);

// Multiplies every sample by gain (0..unity_gain).
void apply_gain(std::span<int16_t> samples, int32_t gain);

}  // namespace avela
