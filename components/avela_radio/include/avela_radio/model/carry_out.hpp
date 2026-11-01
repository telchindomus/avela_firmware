// The one place the radio model's requested fields change: a PlayerRequest carried out on the main loop, by the radio
// service that took it from its queue. Platform-free.
#pragma once

#include "avela_messaging/messages/player_request.hpp"
#include "avela_radio/model/radio_model.hpp"

namespace avela {

void carry_out(const PlayerRequest& request, RadioModel& model);

}  // namespace avela
