// What a radio service provides: it carries out the PlayerRequests its clients queue (into the RadioModel's requested
// fields), makes the requested state happen (tune the station, set the volume, play or pause), reports back what is
// playing and publishes the PlayerStatus (PlayerQueues). Implementations: SimulatedRadioService now; the
// streaming one (Wi-Fi, HTTP stream, decoder, I2S) later. The app picks one at compile time.
#pragma once

#include <concepts>

#include "avela_radio/model/radio_model.hpp"
#include "avela_radio/service/player_queues.hpp"

namespace avela {

template <typename T>
concept RadioServiceConcept = requires(T& service, RadioModel& model) {
    { service.update(model) } -> std::same_as<void>;  // every frame: the requests, then follow them, write the reports
    { service.queues() } -> std::same_as<PlayerQueues&>;
};

}  // namespace avela
