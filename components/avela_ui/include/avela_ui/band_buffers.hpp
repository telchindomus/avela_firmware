#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace avela {

// 10 lines: RAM is the scarce thing (the radio's network buffers need it), a full redraw only runs more, smaller
// bands.
inline constexpr int band_lines = 10;

// The canvas' two band buffers for a screen `Width` pixels wide (RGB565). In static storage (internal RAM, which DMA
// reaches): main declares them for the board's screen, 2 x 6.25 KB at 320 pixels.
template <int Width>
struct BandBuffers {
    alignas(4) std::array<std::array<uint16_t, static_cast<std::size_t>(Width) * band_lines>, 2> m_bands{};
};

}  // namespace avela
