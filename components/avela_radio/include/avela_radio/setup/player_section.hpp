// The player section (setup page and serial console): not settings but the radio's controls: the station (its slot
// number, as in Stations), play or stop, the volume. Reading it shows what the radio does now; applying it hands a
// PlayerRequest to the radio service's queue, which it carries out on the main loop (one frame later); reading takes
// the status it publishes. apply() returns once that status shows the request carried out (at most
// carry_out_timeout), so the page and the console show the new state right after. Runs on the web server's or the
// console's task: talks to the main loop through the queues only, reads station names from Settings.
// The page: the station on its own row; below it Play / Stop (a button that sends `playing` at once, worded as the
// opposite of what the radio does now), the volume and Set (station and volume; Enter in the volume box presses it).
#pragma once

#include <array>
#include <string_view>

#include "avela_messaging/messages/player_request.hpp"
#include "avela_messaging/queues/request_queue.hpp"
#include "avela_messaging/queues/status_queue.hpp"
#include "avela_radio/model/player_status.hpp"
#include "avela_setup/apply_result.hpp"
#include "avela_setup/setup_field.hpp"
#include "avela_setup/setup_section_concept.hpp"
#include "freertos/FreeRTOS.h"

namespace avela {

class PlayerSection {
public:
    static constexpr std::string_view id = "player";
    static constexpr std::string_view title = "Player";
    static constexpr bool writes_own_forms = true;  // its own buttons instead of Save
    static constexpr std::array fields{
        SetupField{
            .m_name = "station", .m_description = "the station's slot, 1..16 (stations lists them)", .m_secret = false},
        SetupField{.m_name = "playing", .m_description = "yes (play) or no (stop)", .m_secret = false},
        SetupField{.m_name = "volume", .m_description = "0..100", .m_secret = false},
    };

    PlayerSection(RequestQueue<PlayerRequest>& requests, const StatusQueue<PlayerStatus>& status)
        : m_requests_(&requests), m_status_(&status)
    {}

    void write_form(HtmlWriter& writer) const;
    [[nodiscard]] ApplyResult apply(const FormFields& submitted) const;
    void write_values(ValueWriter& values) const;

private:
    static constexpr TickType_t carry_out_timeout = pdMS_TO_TICKS(300);  // the main loop takes requests every 33 ms
    static constexpr TickType_t status_poll = pdMS_TO_TICKS(10);

    // Waits until the published status shows `request` carried out; false after carry_out_timeout.
    [[nodiscard]] bool wait_for(const PlayerRequest& request) const;

    // pointers: the server and the console each keep a copy of the section
    RequestQueue<PlayerRequest>* m_requests_;
    const StatusQueue<PlayerStatus>* m_status_;
};

static_assert(SetupSectionConcept<PlayerSection>);

}  // namespace avela
