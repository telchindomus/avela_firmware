// Weather: the playback controls (volume, play/stop), Options = update now.
#pragma once

#include "avela_messaging/messages/weather_update_request.hpp"
#include "avela_ui/command/command.hpp"
#include "avela_ui/controller_concept.hpp"
#include "avela_ui/playback_controls.hpp"
#include "avela_ui/response.hpp"

namespace avela {

class WeatherController : public PlaybackControls {
public:
    using PlaybackControls::handle;

    static Response handle(OptionsCommand /*options*/) { return {.m_request = WeatherUpdateRequest{}}; }
};

static_assert(ControllerConcept<WeatherController>);

}  // namespace avela
