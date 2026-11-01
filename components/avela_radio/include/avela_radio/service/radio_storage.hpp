// Keeps the radio model and the settings in step, on the main loop (init(), update() before the radio service's):
// loads the station list from the StationRecords (again whenever the Stations section saves them), and remembers what
// plays (station, volume, play state) across restarts, so a radio switched off while stopped starts stopped.
#pragma once

#include <cstdint>

#include "avela_radio/model/playback_memory.hpp"
#include "avela_radio/model/radio_model.hpp"
#include "freertos/FreeRTOS.h"

namespace avela {

class RadioStorage {
public:
    // Main loop, after Settings::init(): loads the stations (on the first start it stores the default ones) and what
    // played before the restart.
    void init(RadioModel& model);
    // Main loop, every frame: reloads the stations when they were saved, remembers what plays.
    void update(RadioModel& model);

private:
    static constexpr TickType_t remember_after = pdMS_TO_TICKS(10'000);  // not a flash write per knob detent

    // The used slots into the list; keeps the current station if its slot still holds one.
    void load_stations(RadioModel& model);
    // Saves the station, volume and play state once they stayed the same for remember_after.
    void remember(const RadioModel& model);

    uint32_t m_stations_revision_ = 0;
    PlaybackMemory m_saved_{};     // what is stored
    PlaybackMemory m_changing_{};  // what the radio does, once it differs from m_saved_
    TickType_t m_changed_at_ = 0;
};

}  // namespace avela
