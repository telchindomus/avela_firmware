// Every request a client can send to a service: plain data (one type per file here), so a component can ask another
// for something without depending on it. Each service owns a RequestQueue for its own requests and carries them out
// on the main loop. An optional component's requests are here only when the component is on (menuconfig).
#pragma once

#include <variant>

#include "avela_messaging/messages/accept_uploads_request.hpp"
#include "avela_messaging/messages/player_request.hpp"
#include "avela_messaging/messages/setup_network_request.hpp"
#include "sdkconfig.h"
#if CONFIG_AVELA_WEATHER
#include "avela_messaging/messages/weather_update_request.hpp"
#endif

namespace avela {

#if CONFIG_AVELA_WEATHER
using Request = std::variant<PlayerRequest, SetupNetworkRequest, AcceptUploadsRequest, WeatherUpdateRequest>;
#else
using Request = std::variant<PlayerRequest, SetupNetworkRequest, AcceptUploadsRequest>;
#endif

}  // namespace avela
