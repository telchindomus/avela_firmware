// The radio service's queues: the PlayerRequests its clients send (the knob's controllers, the setup page, the
// console) and the PlayerStatus it publishes for the setup page and the console. Every radio service owns one: its
// update(), on the main loop, takes the requests first and publishes the status last.
#pragma once

#include <optional>

#include "avela_messaging/messages/player_request.hpp"
#include "avela_messaging/queues/request_queue.hpp"
#include "avela_messaging/queues/status_queue.hpp"
#include "avela_radio/model/carry_out.hpp"
#include "avela_radio/model/player_status.hpp"
#include "avela_radio/model/radio_model.hpp"

namespace avela {

class PlayerQueues {
public:
    // Carries out the requests waiting, in order.
    void take_requests(RadioModel& model)
    {
        while (const std::optional<PlayerRequest> request = m_requests_.take()) {
            carry_out(*request, model);
        }
    }

    // What the radio does now, for the other tasks.
    void publish(const RadioModel& model) { m_status_.publish(status_of(model)); }

    RequestQueue<PlayerRequest>& requests() { return m_requests_; }
    [[nodiscard]] const StatusQueue<PlayerStatus>& status() const { return m_status_; }

private:
    RequestQueue<PlayerRequest> m_requests_;
    StatusQueue<PlayerStatus> m_status_;
};

}  // namespace avela
