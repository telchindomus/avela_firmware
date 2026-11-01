// Band renderer: an area of the screen is drawn in horizontal bands of up to 10 lines into one of two buffers, and
// each band is sent to the display by DMA while the next one is drawn. No frame buffer (320x170x2 = 109 KB would not
// fit next to the audio buffers later), no heap: main gives the canvas two band buffers in static storage, sized for
// the board's screen (BandBuffers).
//
// Drawing code always uses screen coordinates; Painter translates them into the current band.
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <initializer_list>
#include <numbers>
#include <span>

#include <LovyanGFX.hpp>               // after <cstdio>: its file wrapper calls fopen() once anything has declared FILE
#include <lgfx/utility/lgfx_qrcode.h>  // the QR encoder LovyanGFX's own qrcode() uses

#include "avela_ui/band_buffers.hpp"
#include "avela_ui/painter.hpp"
#include "avela_ui/rect.hpp"

namespace avela {

class Canvas {
public:
    static constexpr int color_depth_bits = 16;  // RGB565, the panel's native format

    template <int Width>
    Canvas(lgfx::LGFX_Device& display, BandBuffers<Width>& buffers)
        : m_display_(display), m_buffers_{buffers.m_bands[0], buffers.m_bands[1]}
    {}

    // Fills `area` with `background`, runs draw(Painter&) for every band of it and pushes the bands by DMA.
    // `area` is at most as wide as the screen the band buffers were made for.
    template <typename DrawFunction>
    void render(Rect area, uint32_t background, const DrawFunction& draw)
    {
        m_display_.startWrite();
        for (int band_top = area.m_y; band_top < area.bottom(); band_top += band_lines) {
            const int band_height = std::min(band_lines, area.bottom() - band_top);
            lgfx::LGFX_Sprite& sprite = m_sprites_[m_next_buffer_];
            sprite.setBuffer(m_buffers_[m_next_buffer_].data(), area.m_width, band_height, color_depth_bits);
            sprite.fillScreen(background);
            Painter painter{sprite, area.m_x, band_top};
            draw(painter);
            m_display_.waitDMA();  // the other buffer's transfer; this one was freed one band ago
            m_display_.pushImageDMA(area.m_x, band_top, area.m_width, band_height,
                                    static_cast<const lgfx::swap565_t*>(sprite.getBuffer()));
            m_next_buffer_ ^= 1;
        }
        m_display_.endWrite();
    }

private:
    lgfx::LGFX_Device& m_display_;  // whichever panel the board has
    std::array<lgfx::LGFX_Sprite, 2> m_sprites_;
    std::array<std::span<uint16_t>, 2> m_buffers_;
    std::size_t m_next_buffer_ = 0;
};

}  // namespace avela
