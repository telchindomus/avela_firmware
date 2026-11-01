#include "avela_radio/model/player_status.hpp"

namespace avela {

//--------------------------------------------------------------------------------------------------

PlayerStatus status_of(const RadioModel& model)
{
    PlayerStatus status;
    status.m_station_slot = model.m_stations.slot(model.m_station_index);
    status.m_station_name.append(model.station().m_name);
    status.m_playing = model.m_playing;
    status.m_volume = model.m_volume;
    status.m_state = model.m_playback_state;
    return status;
}

}  // namespace avela
