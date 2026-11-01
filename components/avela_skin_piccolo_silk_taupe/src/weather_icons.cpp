#include "avela_skin_piccolo_silk_taupe/weather_icons.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

#include "avela_skin_piccolo_silk_taupe/palette.hpp"

namespace avela::skin_piccolo_silk_taupe {
namespace {

// Every measure is a fraction of the icon's size, so one drawing serves every size.
constexpr float sun_radius = 0.24f;
constexpr float ray_distance = 1.65f;  // of the sun's radius
constexpr float ray_radius = 0.2f;     // of the sun's radius
constexpr int ray_count = 8;
constexpr float moon_radius = 0.3f;
constexpr float moon_cut_x = 0.45f;  // of the moon's radius: where the dark side's circle sits
constexpr float moon_cut_y = -0.35f;
constexpr float moon_cut_radius = 0.85f;

// a cloud of width w: a flat base with three puffs on it, centred
constexpr float cloud_base_height = 0.3f;
constexpr float cloud_left_puff_x = -0.22f;
constexpr float cloud_left_puff_y = 0.02f;
constexpr float cloud_left_puff_radius = 0.2f;
constexpr float cloud_top_puff_x = 0.03f;
constexpr float cloud_top_puff_y = -0.08f;
constexpr float cloud_top_puff_radius = 0.27f;
constexpr float cloud_right_puff_x = 0.25f;
constexpr float cloud_right_puff_y = 0.04f;
constexpr float cloud_right_puff_radius = 0.18f;

constexpr float full_cloud_width = 0.9f;
constexpr float small_sun_offset = -0.13f;  // the sun behind a cloud, up and left
constexpr float small_sun_radius = 0.2f;
constexpr float front_cloud_offset = 0.1f;
constexpr float front_cloud_width = 0.72f;
constexpr float raised_cloud_y = -0.15f;  // above rain, snow or lightning
constexpr float raised_cloud_width = 0.85f;

constexpr float fall_spacing = 0.2f;  // drops and flakes, three in a row
constexpr float drizzle_y = 0.3f;
constexpr float drizzle_radius = 0.05f;
constexpr float rain_top = 0.2f;
constexpr float rain_length = 0.22f;
constexpr float rain_width = 0.07f;
constexpr float snow_y = 0.32f;
constexpr float snow_radius = 0.07f;
constexpr float fog_spacing = 0.2f;
constexpr float fog_width = 0.8f;
constexpr float fog_height = 0.09f;
constexpr float fog_shift = 0.08f;

// the lightning bolt: two triangles under the cloud, corners as fractions of the size from the icon's centre
struct Corner {
    float m_x = 0.0f;
    float m_y = 0.0f;
};
using Triangle = std::array<Corner, 3>;
constexpr Triangle bolt_upper{Corner{.m_x = 0.06f, .m_y = 0.08f}, Corner{.m_x = -0.14f, .m_y = 0.32f},
                              Corner{.m_x = 0.02f, .m_y = 0.3f}};
constexpr Triangle bolt_lower{Corner{.m_x = -0.02f, .m_y = 0.24f}, Corner{.m_x = 0.13f, .m_y = 0.24f},
                              Corner{.m_x = -0.08f, .m_y = 0.5f}};

constexpr int minimum_pixels = 1;

int scaled(int size, float fraction)
{
    return static_cast<int>(std::lround(static_cast<float>(size) * fraction));
}

int scaled_at_least_one(int size, float fraction)
{
    return std::max(minimum_pixels, scaled(size, fraction));
}

void draw_sun(Painter& painter, Point center, int radius)
{
    painter.fill_circle(center.m_x, center.m_y, radius, palette::weather_sun);
    const float distance = static_cast<float>(radius) * ray_distance;
    const int ray = std::max(minimum_pixels, static_cast<int>(std::lround(static_cast<float>(radius) * ray_radius)));
    for (int index = 0; index < ray_count; ++index) {
        const float angle =
            2.0f * std::numbers::pi_v<float> * static_cast<float>(index) / static_cast<float>(ray_count);
        painter.fill_circle(center.m_x + static_cast<int>(std::lround(distance * std::cos(angle))),
                            center.m_y + static_cast<int>(std::lround(distance * std::sin(angle))), ray,
                            palette::weather_sun);
    }
}

void draw_moon(Painter& painter, Point center, int radius, uint32_t background)
{
    painter.fill_circle(center.m_x, center.m_y, radius, palette::weather_sun);
    painter.fill_circle(center.m_x + scaled(radius, moon_cut_x), center.m_y + scaled(radius, moon_cut_y),
                        scaled_at_least_one(radius, moon_cut_radius), background);
}

void draw_sky_light(Painter& painter, Point center, int radius, bool daytime, uint32_t background)
{
    if (daytime) {
        draw_sun(painter, center, radius);
    } else {
        draw_moon(painter, center, radius, background);
    }
}

void draw_cloud(Painter& painter, Point center, int width)
{
    const int base_height = scaled_at_least_one(width, cloud_base_height);
    painter.fill_round_rect(
        Rect{.m_x = center.m_x - width / 2, .m_y = center.m_y, .m_width = width, .m_height = base_height},
        base_height / 2, palette::weather_cloud);
    painter.fill_circle(center.m_x + scaled(width, cloud_left_puff_x), center.m_y + scaled(width, cloud_left_puff_y),
                        scaled_at_least_one(width, cloud_left_puff_radius), palette::weather_cloud);
    painter.fill_circle(center.m_x + scaled(width, cloud_top_puff_x), center.m_y + scaled(width, cloud_top_puff_y),
                        scaled_at_least_one(width, cloud_top_puff_radius), palette::weather_cloud);
    painter.fill_circle(center.m_x + scaled(width, cloud_right_puff_x), center.m_y + scaled(width, cloud_right_puff_y),
                        scaled_at_least_one(width, cloud_right_puff_radius), palette::weather_cloud);
}

// three of something in a row, centred under the icon's middle
template <typename DrawOne>
void three_in_a_row(Point center, int size, const DrawOne& draw_one)
{
    for (int index = -1; index <= 1; ++index) {
        draw_one(center.m_x + index * scaled(size, fall_spacing));
    }
}

void draw_bolt(Painter& painter, Point center, int size, const Triangle& triangle)
{
    const auto [first, second, third] = triangle;
    painter.fill_triangle(center.m_x + scaled(size, first.m_x), center.m_y + scaled(size, first.m_y),
                          center.m_x + scaled(size, second.m_x), center.m_y + scaled(size, second.m_y),
                          center.m_x + scaled(size, third.m_x), center.m_y + scaled(size, third.m_y),
                          palette::weather_sun);
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void draw_weather_icon(Painter& painter, Point center, int size, WeatherCondition condition, bool daytime,
                       uint32_t background)
{
    const Point raised{.m_x = center.m_x, .m_y = center.m_y + scaled(size, raised_cloud_y)};
    switch (condition) {
    case WeatherCondition::clear:
        draw_sky_light(painter, center, scaled_at_least_one(size, daytime ? sun_radius : moon_radius), daytime,
                       background);
        break;
    case WeatherCondition::partly_cloudy:
        draw_sky_light(painter,
                       Point{.m_x = center.m_x + scaled(size, small_sun_offset),
                             .m_y = center.m_y + scaled(size, small_sun_offset)},
                       scaled_at_least_one(size, small_sun_radius), daytime, background);
        draw_cloud(painter,
                   Point{.m_x = center.m_x + scaled(size, front_cloud_offset),
                         .m_y = center.m_y + scaled(size, front_cloud_offset)},
                   scaled_at_least_one(size, front_cloud_width));
        break;
    case WeatherCondition::cloudy:
        draw_cloud(painter, center, scaled_at_least_one(size, full_cloud_width));
        break;
    case WeatherCondition::fog:
        for (int index = -1; index <= 1; ++index) {
            const int height = std::max(2, scaled(size, fog_height));
            const int width = scaled_at_least_one(size, fog_width);
            painter.fill_round_rect(Rect{.m_x = center.m_x - width / 2 + index * scaled(size, fog_shift),
                                         .m_y = center.m_y + index * scaled(size, fog_spacing) - height / 2,
                                         .m_width = width,
                                         .m_height = height},
                                    height / 2, palette::weather_cloud);
        }
        break;
    case WeatherCondition::drizzle:
        draw_cloud(painter, raised, scaled_at_least_one(size, raised_cloud_width));
        three_in_a_row(center, size, [&](int x) {
            painter.fill_circle(x, center.m_y + scaled(size, drizzle_y), scaled_at_least_one(size, drizzle_radius),
                                palette::weather_precipitation);
        });
        break;
    case WeatherCondition::rain:
        draw_cloud(painter, raised, scaled_at_least_one(size, raised_cloud_width));
        three_in_a_row(center, size, [&](int x) {
            const int width = std::max(2, scaled(size, rain_width));
            painter.fill_round_rect(Rect{.m_x = x - width / 2,
                                         .m_y = center.m_y + scaled(size, rain_top),
                                         .m_width = width,
                                         .m_height = scaled_at_least_one(size, rain_length)},
                                    width / 2, palette::weather_precipitation);
        });
        break;
    case WeatherCondition::snow:
        draw_cloud(painter, raised, scaled_at_least_one(size, raised_cloud_width));
        three_in_a_row(center, size, [&](int x) {
            painter.fill_circle(x, center.m_y + scaled(size, snow_y), scaled_at_least_one(size, snow_radius),
                                palette::weather_cloud);
        });
        break;
    case WeatherCondition::thunderstorm:
        draw_cloud(painter, raised, scaled_at_least_one(size, raised_cloud_width));
        draw_bolt(painter, center, size, bolt_upper);
        draw_bolt(painter, center, size, bolt_lower);
        break;
    }
}

}  // namespace avela::skin_piccolo_silk_taupe
