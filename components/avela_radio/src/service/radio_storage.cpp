#include "avela_radio/service/radio_storage.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>

#include "avela_radio/model/default_station.hpp"
#include "avela_settings/settings.hpp"
#include "esp_log.h"
#include "freertos/task.h"

namespace avela {
namespace {

constexpr const char* log_tag = "radio";

PlaybackMemory memory_of(const RadioModel& model)
{
    return PlaybackMemory{.m_station_slot = model.m_stations.slot(model.m_station_index),
                          .m_volume = static_cast<int8_t>(model.m_volume),
                          .m_playing = model.m_playing};
}

// Whether any slot holds a station.
bool any_station_stored()
{
    for (std::size_t slot = 1; slot <= StationRecord::slot_count; ++slot) {
        StationRecord record;
        if (Settings::load(record, slot) && !record.m_url.empty()) {
            return true;
        }
    }
    return false;
}

void store_default_stations()
{
    std::size_t slot = 1;
    for (const DefaultStation& station : default_stations) {
        StationRecord record;
        record.m_name.append(station.m_name);
        record.m_genre.append(station.m_genre);
        record.m_url.append(station.m_url);
        Settings::save(record, slot++);
    }
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void RadioStorage::init(RadioModel& model)
{
    if (!any_station_stored()) {
        ESP_LOGI(log_tag, "first start: storing the default stations");
        store_default_stations();
    }
    load_stations(model);

    if (Settings::load(m_saved_) && m_saved_.m_station_slot != 0) {
        model.m_station_index = model.m_stations.index_of(m_saved_.m_station_slot).value_or(0);
        model.m_volume = std::clamp(int{m_saved_.m_volume}, RadioModel::volume_min, RadioModel::volume_max);
        model.m_playing = m_saved_.m_playing;
    }
    m_changing_ = memory_of(model);
}

//--------------------------------------------------------------------------------------------------

void RadioStorage::update(RadioModel& model)
{
    if (Settings::revision<StationRecord>() != m_stations_revision_) {
        load_stations(model);
    }
    remember(model);
}

//--------------------------------------------------------------------------------------------------

void RadioStorage::load_stations(RadioModel& model)
{
    m_stations_revision_ = Settings::revision<StationRecord>();
    const uint8_t current_slot = model.m_stations.slot(model.m_station_index);
    model.m_stations.clear();
    for (std::size_t slot = 1; slot <= StationRecord::slot_count; ++slot) {
        StationRecord record;
        if (Settings::load(record, slot) && !record.m_url.empty()) {
            model.m_stations.add(static_cast<uint8_t>(slot), record.m_name.view(), record.m_genre.view());
        }
    }
    if (model.m_stations.size() == 0) {
        // nothing readable (the Stations section never leaves the list empty): the defaults, unsaved
        std::size_t slot = 1;
        for (const DefaultStation& station : default_stations) {
            model.m_stations.add(static_cast<uint8_t>(slot++), station.m_name, station.m_genre);
        }
    }
    model.m_station_index = model.m_stations.index_of(current_slot).value_or(0);
}

//--------------------------------------------------------------------------------------------------

void RadioStorage::remember(const RadioModel& model)
{
    const TickType_t now = xTaskGetTickCount();
    const PlaybackMemory current = memory_of(model);
    if (current != m_changing_) {
        m_changing_ = current;
        m_changed_at_ = now;
    }
    if (m_changing_ != m_saved_ && now - m_changed_at_ >= remember_after) {
        if (Settings::save(m_changing_)) {
            m_saved_ = m_changing_;
        } else {
            m_changed_at_ = now;  // NVS refused (logged): try again later
        }
    }
}

}  // namespace avela
