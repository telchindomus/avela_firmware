// Now playing: hands the radio model's station, stream, play state, playback state and spectrum to its skin (the
// header shows the volume and the play state on every screen: avela_status_tray).
#pragma once

#include <utility>

#include "avela_radio/model/radio_model.hpp"
#include "avela_radio/ui/player_skin_concept.hpp"

namespace avela {

template <PlayerSkinConcept PageSkin>
class PlayerView {
public:
    PlayerView(const RadioModel& model, PageSkin skin) : m_model_(model), m_skin_(std::move(skin)) {}

    void draw()
    {
        refresh();
        m_skin_.draw();
    }

    void refresh()
    {
        const Station& station = m_model_.station();
        m_skin_.set_station(station.m_name, station.m_genre);
        m_skin_.set_thumbnail(Thumbnail{.m_monogram = station.m_monogram});
        m_skin_.set_stream(m_model_.m_stream_title.view(), m_model_.m_stream_format.view());
        m_skin_.set_playing(m_model_.m_playing);
        m_skin_.set_playback_state(m_model_.m_playback_state);
        m_skin_.set_spectrum(m_model_.m_spectrum);
    }

private:
    const RadioModel& m_model_;
    PageSkin m_skin_;
};

}  // namespace avela
