// The models the Ui builds its screens from, owned by the Application (one each, in static storage). Read only: the
// views and controllers never write a data model; what they want changed they ask for with a request. Adding a
// component's screens = its model here, its screens (in its ui/ folder) in Ui (the variant, build()), their page skins
// in every skin; an optional component's under its switch (menuconfig).
#pragma once

#include "avela_firmware_update/model/firmware_update_model.hpp"
#include "avela_network/model/network_model.hpp"
#include "avela_radio/model/radio_model.hpp"
#include "avela_system_info/model/system_info_model.hpp"
#include "sdkconfig.h"
#if CONFIG_AVELA_WEATHER
#include "avela_weather/model/weather_model.hpp"
#endif

namespace avela {

struct UiModels {
    const RadioModel& m_radio;
    const SystemInfoModel& m_system_info;
    const NetworkModel& m_network;
    const FirmwareUpdateModel& m_firmware_update;
#if CONFIG_AVELA_WEATHER
    const WeatherModel& m_weather;
#endif
};

}  // namespace avela
