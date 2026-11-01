// What drawing code draws with, in screen coordinates; it translates them into the current band (Canvas).
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

#include "avela_ui/point.hpp"
#include "avela_ui/rect.hpp"

namespace avela {

// Ring angles in degrees: 0 = 3 o'clock, clockwise.
inline constexpr float full_turn_degrees = 360.0f;
inline constexpr float twelve_o_clock_degrees = -90.0f;

// How much of a pixel a shape covers, for blending: 0 = none (background), full_coverage = all (foreground).
inline constexpr uint32_t full_coverage = 255;

// Mixes two 0xRRGGBB colours by coverage (0..full_coverage).
constexpr uint32_t blend(uint32_t background, uint32_t foreground, uint32_t coverage)
{
    constexpr uint32_t channel_mask = 0xFF;
    constexpr uint32_t red_shift = 16;
    constexpr uint32_t green_shift = 8;
    constexpr uint32_t blue_shift = 0;
    uint32_t mixed = 0;
    for (const uint32_t shift : {red_shift, green_shift, blue_shift}) {
        const uint32_t back = (background >> shift) & channel_mask;
        const uint32_t fore = (foreground >> shift) & channel_mask;
        mixed |= ((back * (full_coverage - coverage) + fore * coverage + full_coverage / 2) / full_coverage) << shift;
    }
    return mixed;
}

// Colours are 0xRRGGBB (uint32_t, which LovyanGFX treats as RGB888).
class Painter {
public:
    Painter(lgfx::LGFX_Sprite& sprite, int origin_x, int origin_y)
        : m_sprite_(sprite), m_origin_x_(origin_x), m_origin_y_(origin_y)
    {}

    void fill_rect(Rect area, uint32_t color)
    {
        m_sprite_.fillRect(area.m_x - m_origin_x_, area.m_y - m_origin_y_, area.m_width, area.m_height, color);
    }

    void fill_round_rect(Rect area, int radius, uint32_t color)
    {
        m_sprite_.fillSmoothRoundRect(area.m_x - m_origin_x_, area.m_y - m_origin_y_, area.m_width, area.m_height,
                                      radius, color);
    }

    void fill_circle(int center_x, int center_y, int radius, uint32_t color)
    {
        m_sprite_.fillSmoothCircle(center_x - m_origin_x_, center_y - m_origin_y_, radius, color);
    }

    void fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
    {
        m_sprite_.fillTriangle(x0 - m_origin_x_, y0 - m_origin_y_, x1 - m_origin_x_, y1 - m_origin_y_, x2 - m_origin_x_,
                               y2 - m_origin_y_, color);
    }

    void horizontal_line(int x, int y, int width, uint32_t color)
    {
        m_sprite_.drawFastHLine(x - m_origin_x_, y - m_origin_y_, width, color);
    }

    // Anti-aliased ring segment over a known background. Angles in degrees, 0 = 3 o'clock, clockwise.
    void ring(float center_x, float center_y, float outer_radius, float thickness, float start_degrees,
              float sweep_degrees, uint32_t color, uint32_t background)
    {
        const float inner_radius = outer_radius - thickness;
        const int left = static_cast<int>(std::floor(center_x - outer_radius - 1));
        const int right = static_cast<int>(std::ceil(center_x + outer_radius + 1));
        const int top = std::max(static_cast<int>(std::floor(center_y - outer_radius - 1)), m_origin_y_);
        const int bottom = std::min(static_cast<int>(std::ceil(center_y + outer_radius + 1)),
                                    m_origin_y_ + static_cast<int>(m_sprite_.height()));
        const bool full_circle = sweep_degrees >= full_turn_degrees;
        for (int y = top; y < bottom; ++y) {
            for (int x = left; x < right; ++x) {
                const float dx = static_cast<float>(x) + 0.5f - center_x;
                const float dy = static_cast<float>(y) + 0.5f - center_y;
                const float distance = std::sqrt(dx * dx + dy * dy);
                const float coverage =
                    std::clamp(std::min(outer_radius - distance, distance - inner_radius) + 0.5f, 0.0f, 1.0f);
                if (coverage <= 0.0f) {
                    continue;
                }
                if (!full_circle) {
                    float angle = std::atan2(dy, dx) * degrees_per_radian - start_degrees;
                    angle = std::fmod(angle, full_turn_degrees);
                    if (angle < 0.0f) {
                        angle += full_turn_degrees;
                    }
                    if (angle > sweep_degrees) {
                        continue;
                    }
                }
                const auto covered = static_cast<uint32_t>(std::lround(coverage * static_cast<float>(full_coverage)));
                m_sprite_.drawPixel(x - m_origin_x_, y - m_origin_y_, blend(background, color, covered));
            }
        }
    }

    void text(const char* string, int x, int y, const lgfx::IFont& font, uint32_t color,
              lgfx::textdatum_t datum = lgfx::textdatum_t::top_left)
    {
        m_sprite_.setTextColor(color);
        m_sprite_.setTextDatum(datum);
        m_sprite_.drawString(string, x - m_origin_x_, y - m_origin_y_, &font);
    }

    [[nodiscard]] int text_width(const char* string, const lgfx::IFont& font)
    {
        return m_sprite_.textWidth(string, &font);
    }

    void image_with_alpha(int x, int y, int width, int height, const lgfx::argb8888_t* pixels)
    {
        m_sprite_.pushAlphaImage(x - m_origin_x_, y - m_origin_y_, width, height, pixels);
    }

    // QR code in `area` (a square, area.m_width wide): `dark` modules on a `light` square with rounded corners, as big
    // as whole modules allow, centred (what is left over is the margin; the corner radius is kept within it, so the
    // corners never cut a module). The version is the smallest from `first_version` up to
    // max_qr_version that holds the text (error correction low, like LovyanGFX's qrcode()); nothing is drawn if
    // none does. Scanners need dark modules clearly darker than the light ones.
    void qr_code(const char* text, Rect area, uint8_t first_version, uint32_t dark, uint32_t light, int corner_radius)
    {
        std::array<uint8_t, qr_buffer_bytes> modules{};
        for (uint8_t version = first_version; version <= max_qr_version; ++version) {
            QRCode code{};
            if (lgfx_qrcode_initText(&code, modules.data(), version, qr_error_correction_low, text) != 0) {
                continue;
            }
            const int size = code.size;
            const int module = area.m_width / size;
            const int offset = (area.m_width - (size * module)) / 2;
            fill_round_rect(Rect{.m_x = area.m_x, .m_y = area.m_y, .m_width = area.m_width, .m_height = area.m_width},
                            std::min(corner_radius, offset), light);
            for (int row = 0; row < size; ++row) {
                for (int column = 0; column < size; ++column) {
                    if (lgfx_qrcode_getModule(&code, static_cast<uint_fast8_t>(column),
                                              static_cast<uint_fast8_t>(row))) {
                        fill_rect(Rect{.m_x = area.m_x + offset + (column * module),
                                       .m_y = area.m_y + offset + (row * module),
                                       .m_width = module,
                                       .m_height = module},
                                  dark);
                    }
                }
            }
            return;
        }
    }

    // Restricts drawing to an area (screen coordinates) until clear_clip().
    void clip(Rect area)
    {
        m_sprite_.setClipRect(area.m_x - m_origin_x_, area.m_y - m_origin_y_, area.m_width, area.m_height);
    }
    void clear_clip() { m_sprite_.clearClipRect(); }

private:
    static constexpr float degrees_per_radian = 180.0f / std::numbers::pi_v<float>;

    // QR codes up to version 10 (57 x 57 modules, up to 271 bytes of text at low error correction: the longest Wi-Fi
    // join code is about 120); the module bits are kept on the stack while drawing.
    static constexpr uint8_t max_qr_version = 10;
    static constexpr int qr_version_step = 4;     // modules per side added per version
    static constexpr int qr_version_1_size = 21;  // modules per side at version 1
    static constexpr int qr_max_size = qr_version_1_size + (qr_version_step * (max_qr_version - 1));
    static constexpr std::size_t qr_buffer_bytes = ((qr_max_size * qr_max_size) + 7) / 8;  // one bit per module
    static constexpr uint8_t qr_error_correction_low = 0;

    lgfx::LGFX_Sprite& m_sprite_;
    int m_origin_x_;
    int m_origin_y_;
};

}  // namespace avela
