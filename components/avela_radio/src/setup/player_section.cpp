#include "avela_radio/setup/player_section.hpp"

#include <optional>

#include "avela_radio/model/station_record.hpp"
#include "avela_settings/settings.hpp"
#include "avela_setup/setup_server.hpp"
#include "avela_setup/whole_number.hpp"
#include "freertos/task.h"

namespace avela {
namespace {

using Number = FixedString<number_capacity<int>>;

std::string_view state_text(PlaybackState state)
{
    switch (state) {
    case PlaybackState::stopped:
        return "stopped";
    case PlaybackState::connecting:
        return "connecting";
    case PlaybackState::playing:
        return "playing";
    case PlaybackState::failed:
        return "the station doesn't answer";
    }
    return "";
}

bool holds_station(std::size_t slot)
{
    StationRecord record;
    return Settings::load(record, slot) && !record.m_url.empty();
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void PlayerSection::write_form(HtmlWriter& writer) const
{
    const PlayerStatus status = m_status_->read();
    writer.raw("<p>Now: ").text(status.m_station_name.view()).raw(", ").text(state_text(status.m_state)).raw("</p>");
    setup_page::write_form_start(writer, id);
    writer.raw(R"(<label for="station">Station</label><select id="station" name="station">)");
    for (std::size_t slot = 1; slot <= StationRecord::slot_count; ++slot) {
        StationRecord record;
        if (!Settings::load(record, slot) || record.m_url.empty()) {
            continue;
        }
        Number value;
        value.append_number(slot);
        writer.raw(R"(<option value=")")
            .text(value.view())
            .raw(slot == status.m_station_slot ? R"(" selected>)" : R"(">)");
        writer.text(record.m_name.view()).raw("</option>");
    }
    Number volume;
    volume.append_number(status.m_volume);
    writer.raw(R"(</select><label for="volume">Volume</label><div class="row">)");
    writer.raw(R"(<input id="volume" name="volume" type="number" min="0" max="100" value=")").text(volume.view());
    // Set comes first, so Enter in the volume box presses it; Play / Stop is shown first (class lead)
    writer.raw(R"("><button type="submit">Set</button>)");
    writer.raw(status.m_playing
                   ? R"(<button type="submit" class="lead" name="playing" value="no">&#9632; Stop</button>)"
                   : R"(<button type="submit" class="lead" name="playing" value="yes">&#9654; Play</button>)");
    writer.raw("</div></form>");
}

//--------------------------------------------------------------------------------------------------

void PlayerSection::write_values(ValueWriter& values) const
{
    const PlayerStatus status = m_status_->read();
    Number station;
    station.append_number(status.m_station_slot);
    Number volume;
    volume.append_number(status.m_volume);
    values.value("station", station.view());
    values.value("playing", status.m_playing ? "yes" : "no");
    values.value("volume", volume.view());
    FixedString<capacity_for("now: , the station doesn't answer") + StationRecord::name_capacity> now;
    now.append("now: ").append(status.m_station_name.view()).append(", ").append(state_text(status.m_state));
    values.note(now.view());
}

//--------------------------------------------------------------------------------------------------

ApplyResult PlayerSection::apply(const FormFields& submitted) const
{
    PlayerRequest request;
    FixedString<ApplyResult::message_capacity> done;
    if (const std::optional<std::string_view> station = submitted.find("station")) {
        const std::optional<int> slot = whole_number(*station);
        if (!slot || *slot < 1 || *slot > static_cast<int>(StationRecord::slot_count) ||
            !holds_station(static_cast<std::size_t>(*slot))) {
            return ApplyResult::invalid("station", "No station in that slot (stations lists them).");
        }
        request.m_station_slot = static_cast<uint8_t>(*slot);
        done.append("station ").append_number(*slot);
    }
    if (const std::optional<std::string_view> playing = submitted.find("playing")) {
        if (*playing != "yes" && *playing != "no") {
            return ApplyResult::invalid("playing", "Playing is yes or no.");
        }
        request.m_playing = *playing == "yes";
        done.append(done.empty() ? "" : ", ").append(*request.m_playing ? "playing" : "stopped");
    }
    if (const std::optional<std::string_view> volume = submitted.find("volume")) {
        const std::optional<int> level = whole_number(*volume);
        if (!level || *level < RadioModel::volume_min || *level > RadioModel::volume_max) {
            return ApplyResult::invalid("volume", "The volume is a whole number, 0..100.");
        }
        request.m_volume = level;
        done.append(done.empty() ? "" : ", ").append("volume ").append_number(*level);
    }
    if (done.empty()) {
        return ApplyResult::invalid("", "Nothing to do: give station, playing or volume.");
    }
    if (!m_requests_->send(request)) {
        return ApplyResult::invalid("", "The radio is busy; please try again.");
    }
    ApplyResult result;
    result.m_saved = true;
    result.m_message.append(wait_for(request) ? "Done: " : "Sent, not carried out yet: ")
        .append(done.view())
        .append(".");
    return result;
}

//--------------------------------------------------------------------------------------------------

bool PlayerSection::wait_for(const PlayerRequest& request) const
{
    const auto carried_out = [&] {
        const PlayerStatus status = m_status_->read();
        return (request.m_station_slot == 0 || status.m_station_slot == request.m_station_slot) &&
               (!request.m_playing || status.m_playing == *request.m_playing) &&
               (!request.m_volume || status.m_volume == *request.m_volume);
    };
    const TickType_t started = xTaskGetTickCount();
    while (!carried_out()) {
        if (xTaskGetTickCount() - started >= carry_out_timeout) {
            return false;
        }
        vTaskDelay(status_poll);
    }
    return true;
}

}  // namespace avela
