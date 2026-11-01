// The right part of the header: hands the local time (SystemClock), the radio's play state and volume (RadioModel)
// and the Wi-Fi status (NetworkModel) to its skin, which draws them in the skin's header. Until anything sets the
// system clock the time is unsynchronised (the skin shows no time).
#pragma once

#include <utility>

#include "avela_clock/system_clock.hpp"
#include "avela_network/model/network_model.hpp"
#include "avela_radio/model/radio_model.hpp"
#include "avela_status_tray/status_tray_skin_concept.hpp"

namespace avela {

template <StatusTraySkinConcept PageSkin>
class StatusTrayView {
public:
    StatusTrayView(const RadioModel& radio, const NetworkModel& network, PageSkin skin)
        : m_radio_(radio), m_network_(network), m_skin_(std::move(skin))
    {}

    void draw()
    {
        refresh();
        m_skin_.draw();
    }

    void refresh()
    {
        m_skin_.set_time(SystemClock::now());
        m_skin_.set_playback(m_radio_.m_playing, m_radio_.m_playback_state);
        m_skin_.set_volume(m_radio_.m_volume);
        m_skin_.set_wifi(m_network_.m_status, m_network_.signal_bars());
    }

private:
    const RadioModel& m_radio_;
    const NetworkModel& m_network_;
    PageSkin m_skin_;
};

}  // namespace avela
