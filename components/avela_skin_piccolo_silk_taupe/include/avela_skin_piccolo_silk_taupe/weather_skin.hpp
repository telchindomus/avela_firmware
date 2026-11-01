// The Silk Taupe skin's weather screen: the big icon and temperature, the condition, "feels · wind",
// today's high and low, the place, humidity as a ring, and the next four days below the footer line. Without values
// (no place set, no network yet, place not found, first update failed) a headline and a hint say why; with older
// values the place line says what is going on. Redraws whole when the report's revision changes (every few minutes
// at most). Fulfils WeatherSkin (avela_weather).
#pragma once

#include <string_view>

#include "avela_network/model/host_name.hpp"
#include "avela_skin_piccolo_silk_taupe/surface.hpp"
#include "avela_skin_piccolo_silk_taupe/widgets.hpp"
#include "avela_weather/model/weather_report.hpp"
#include "avela_weather/ui/weather_skin_concept.hpp"

namespace avela::skin_piccolo_silk_taupe {

class WeatherSkin {
public:
    explicit WeatherSkin(const Surface& surface) : m_surface_(surface) {}

    void set_weather(const WeatherReport& report);
    void set_host_name(std::string_view host_name);
    void draw();

private:
    void render() const;
    void draw_values(Painter& painter) const;
    void draw_forecast(Painter& painter) const;
    void draw_message(Painter& painter) const;

    Surface m_surface_;
    bool m_shown_ = false;
    bool m_has_report_ = false;
    WeatherReport m_report_;
    FixedString<HostName::capacity> m_host_name_;
};

}  // namespace avela::skin_piccolo_silk_taupe

namespace avela {
static_assert(WeatherSkinConcept<skin_piccolo_silk_taupe::WeatherSkin>);
}  // namespace avela
