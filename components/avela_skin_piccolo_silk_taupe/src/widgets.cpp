#include "avela_skin_piccolo_silk_taupe/widgets.hpp"

#include <algorithm>
#include <cstdlib>

#include "avela_skin_piccolo_silk_taupe/header_layout.hpp"
#include "avela_skin_piccolo_silk_taupe/palette.hpp"
#include "avela_skin_piccolo_silk_taupe/screen.hpp"

namespace avela::skin_piccolo_silk_taupe {

namespace {

// A light stripe across gold: its centre line crosses the shape's middle row at `m_center_numerator /
// m_center_denominator` of its width and leans right as it rises; its colour goes from the accent to
// palette::gold_reflection, strongest (m_strength of full_coverage) on the centre line, fading over m_half_width
// pixels to each side.
struct Reflection {
    int m_center_numerator = 0;
    int m_center_denominator = 1;
    int m_half_width = 0;
    uint32_t m_strength = 0;
};

constexpr int lean_rows_per_pixel = 2;  // the centre line moves one pixel right every two rows up

// The broad main reflection: at two fifths of the width, a sixth of the width to each side (at least 3 pixels).
constexpr int main_center_numerator = 2;
constexpr int main_center_denominator = 5;
constexpr int main_half_width_divisor = 6;
constexpr int main_minimum_half_width = 3;
constexpr uint32_t main_strength = 100;

// The badges' quieter second reflection at three quarters of the width, balancing the main one on the pill.
constexpr Reflection badge_second_reflection{
    .m_center_numerator = 3, .m_center_denominator = 4, .m_half_width = 2, .m_strength = 65};

Reflection main_reflection(Rect area)
{
    return Reflection{.m_center_numerator = main_center_numerator,
                      .m_center_denominator = main_center_denominator,
                      .m_half_width = std::max(area.m_width / main_half_width_divisor, main_minimum_half_width),
                      .m_strength = main_strength};
}

// Whether pixel (x, y) is inside the rounded shape, at least a pixel in from its edge, so the smooth edge the shape
// was drawn with keeps its anti-aliasing.
bool inside_rounded(Rect area, int radius, int x, int y)
{
    if (x < area.m_x || x >= area.right() || y < area.m_y || y >= area.bottom()) {
        return false;
    }
    constexpr float pixel_center = 0.5f;
    const float center_x = static_cast<float>(x) + pixel_center;
    const float center_y = static_cast<float>(y) + pixel_center;
    const auto corner_left = static_cast<float>(area.m_x + radius);
    const auto corner_right = static_cast<float>(area.right() - radius);
    const auto corner_top = static_cast<float>(area.m_y + radius);
    const auto corner_bottom = static_cast<float>(area.bottom() - radius);
    const float dx = std::max({corner_left - center_x, 0.0f, center_x - corner_right});
    const float dy = std::max({corner_top - center_y, 0.0f, center_y - corner_bottom});
    const float inner_radius = std::max(static_cast<float>(radius) - 1.0f, 0.0f);
    return (dx * dx) + (dy * dy) <= inner_radius * inner_radius;
}

void draw_reflection(Painter& painter, Rect area, int radius, const Reflection& reflection)
{
    const int center_x = area.m_x + (area.m_width * reflection.m_center_numerator / reflection.m_center_denominator);
    const int middle_row = area.m_height / 2;
    const auto fade_steps = static_cast<uint32_t>(reflection.m_half_width + 1);
    for (int row = 0; row < area.m_height; ++row) {
        const int line_x = center_x + ((middle_row - row) / lean_rows_per_pixel);
        const int y = area.m_y + row;
        for (int offset = -reflection.m_half_width; offset <= reflection.m_half_width; ++offset) {
            const int x = line_x + offset;
            if (!inside_rounded(area, radius, x, y)) {
                continue;
            }
            const auto strength =
                reflection.m_strength * (fade_steps - static_cast<uint32_t>(std::abs(offset))) / fade_steps;
            painter.fill_rect(Rect{.m_x = x, .m_y = y, .m_width = 1, .m_height = 1},
                              blend(palette::accent, palette::gold_reflection, strength));
        }
    }
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void draw_gold_glint(Painter& painter, Rect area, int radius)
{
    draw_reflection(painter, area, radius, main_reflection(area));
}

//--------------------------------------------------------------------------------------------------

void draw_badge(Painter& painter, Rect area, const char* text, const Fonts& fonts)
{
    const int radius = area.m_height / 2;
    painter.fill_round_rect(area, radius, palette::accent);
    draw_gold_glint(painter, area, radius);
    draw_reflection(painter, area, radius, badge_second_reflection);
    painter.text(text, area.m_x + area.m_width / 2, area.m_y + area.m_height / 2, fonts.m_badge.get(),
                 palette::on_accent, lgfx::textdatum_t::middle_center);
}

//--------------------------------------------------------------------------------------------------

void draw_footer_divider(Painter& painter)
{
    painter.horizontal_line(0, layout::footer_divider_y, screen.m_width, palette::divider);
}

}  // namespace avela::skin_piccolo_silk_taupe
