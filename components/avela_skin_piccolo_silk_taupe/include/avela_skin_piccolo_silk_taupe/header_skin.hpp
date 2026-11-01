// The Silk Taupe skin's header band: the logo, "Avela" and the divider line, with the status tray on the right, from
// right to left: clock and date (none until the clock is set); the Wi-Fi icon (arcs lit by signal strength when
// joined, lighting up one after another while joining, all unlit without a network); the volume as six rising bars;
// LIVE with an antenna (a pill while play is wanted, waves going out from the antenna to both sides while sound comes
// out: ((|)); an empty frame while stopped). The icons in the accent (lit) and the volume track's colour (unlit).
// Fulfils StatusTraySkinConcept (avela_status_tray).
#pragma once

#include "avela_clock/local_time.hpp"
#include "avela_network/model/wifi_status.hpp"
#include "avela_radio/model/playback_state.hpp"
#include "avela_skin_piccolo_silk_taupe/surface.hpp"
#include "avela_status_tray/status_tray_skin_concept.hpp"

namespace avela::skin_piccolo_silk_taupe {

class HeaderSkin {
public:
    explicit HeaderSkin(const Surface& surface) : m_surface_(surface) {}

    void set_time(const LocalTime& time);
    void set_playback(bool playing, PlaybackState state);
    void set_volume(int volume);
    void set_wifi(WifiStatus status, int bars);
    void draw();

private:
    void render() const;
    void draw_brand(Painter& painter) const;
    void draw_tray(Painter& painter) const;

    Surface m_surface_;
    bool m_shown_ = false;
    LocalTime m_time_;
    bool m_playing_ = false;
    int m_waves_ = 0;  // what LIVE shows: the waves of the animation step while sound comes out, none otherwise
    int m_volume_ = 0;
    WifiStatus m_wifi_status_ = WifiStatus::not_configured;
    int m_lit_arcs_ = 0;  // what the icon shows: the signal bars when joined, the animation step while joining
};

}  // namespace avela::skin_piccolo_silk_taupe

namespace avela {
static_assert(StatusTraySkinConcept<skin_piccolo_silk_taupe::HeaderSkin>);
}  // namespace avela
