// The firmware update screen (Info's child, Menu there): the playback controls (volume, play/stop); shown, it asks
// for uploads to be accepted; left, it refuses them again. It can't be left while an image is being written or the
// radio is restarting into it (leaving wouldn't stop it).
#pragma once

#include "avela_firmware_update/model/firmware_update_model.hpp"
#include "avela_messaging/messages/accept_uploads_request.hpp"
#include "avela_messaging/messages/request.hpp"
#include "avela_ui/command/command.hpp"
#include "avela_ui/controller_concept.hpp"
#include "avela_ui/playback_controls.hpp"
#include "avela_ui/response.hpp"

namespace avela {

class FirmwareUpdateController : public PlaybackControls {
public:
    explicit FirmwareUpdateController(const FirmwareUpdateModel& model) : m_model_(model) {}

    static Request shown() { return AcceptUploadsRequest{.m_accept = true}; }

    static Request left() { return AcceptUploadsRequest{.m_accept = false}; }

    [[nodiscard]] bool can_leave() const
    {
        return m_model_.m_state != FirmwareUpdateState::receiving &&
               m_model_.m_state != FirmwareUpdateState::restarting;
    }

private:
    const FirmwareUpdateModel& m_model_;
};

static_assert(ControllerConcept<FirmwareUpdateController>);

}  // namespace avela
