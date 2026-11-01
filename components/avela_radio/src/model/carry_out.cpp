#include "avela_radio/model/carry_out.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>

namespace avela {

//--------------------------------------------------------------------------------------------------

void carry_out(const PlayerRequest& request, RadioModel& model)
{
    if (request.m_station_slot != 0) {
        if (const std::optional<std::size_t> index = model.m_stations.index_of(request.m_station_slot)) {
            model.m_station_index = *index;
        }
    }
    if (request.m_playing) {
        model.m_playing = *request.m_playing;
    }
    if (request.m_toggle_playing) {
        model.m_playing = !model.m_playing;
    }
    const int volume = request.m_volume.value_or(model.m_volume) + request.m_volume_change;
    model.m_volume = std::clamp(volume, RadioModel::volume_min, RadioModel::volume_max);
}

}  // namespace avela
