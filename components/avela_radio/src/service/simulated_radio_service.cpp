#include "avela_radio/service/simulated_radio_service.hpp"

#include <array>

namespace avela {
namespace {

struct SimulatedStream {
    const char* m_title;
    const char* m_format;
};

// Used in turn for the stations in the list (station index modulo the count).
constexpr std::array simulated_streams{
    SimulatedStream{.m_title = "Bonobo – Kerala", .m_format = "128 kbit/s MP3"},
    SimulatedStream{.m_title = "Pink Floyd – Time", .m_format = "128 kbit/s MP3"},
    SimulatedStream{.m_title = "Miles Davis – So What", .m_format = "96 kbit/s AAC"},
    SimulatedStream{.m_title = "Nina Simone – Feeling Good", .m_format = "128 kbit/s MP3"},
    SimulatedStream{.m_title = "Boards of Canada – Dayvan Cowboy", .m_format = "128 kbit/s MP3"},
};

}  // namespace

//--------------------------------------------------------------------------------------------------

void SimulatedRadioService::update(RadioModel& model)
{
    m_queues_.take_requests(model);
    report_stream(model);
    report_spectrum(model);
    m_queues_.publish(model);
}

//--------------------------------------------------------------------------------------------------

void SimulatedRadioService::report_stream(RadioModel& model)
{
    if (model.m_station_index == m_reported_station_) {
        return;
    }
    const SimulatedStream& stream = simulated_streams[model.m_station_index % simulated_streams.size()];
    model.m_stream_title.clear().append(stream.m_title);
    model.m_stream_format.clear().append(stream.m_format);
    m_reported_station_ = model.m_station_index;
}

//--------------------------------------------------------------------------------------------------

void SimulatedRadioService::report_spectrum(RadioModel& model)
{
    const int volume_scale = quiet_scale + (per_mille - quiet_scale) * (model.m_volume - RadioModel::volume_min) /
                                               (RadioModel::volume_max - RadioModel::volume_min);
    const int last = static_cast<int>(model.m_spectrum.size()) - 1;
    for (int band = 0; band <= last; ++band) {
        const int distance_from_peak = band * per_mille / last - shape_peak;
        const int shape =
            per_mille - shape_curvature * distance_from_peak * distance_from_peak / (per_mille * per_mille);
        const int randomness =
            minimum_randomness + (per_mille - minimum_randomness) * m_random_per_mille_(m_random_engine_) / per_mille;
        const int level = model.m_playing ? shape * randomness / per_mille * volume_scale / per_mille : 0;
        model.m_spectrum.at(static_cast<std::size_t>(band)) =
            static_cast<uint8_t>(level * RadioModel::spectrum_level_max / per_mille);
    }
}

}  // namespace avela
