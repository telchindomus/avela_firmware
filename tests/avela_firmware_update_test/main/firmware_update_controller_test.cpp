#include "avela_firmware_update/ui/firmware_update_controller.hpp"

#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <variant>

#include "avela_ui/controller_calls.hpp"

namespace avela {
namespace {

TEST_CASE("shown, the screen asks for uploads to be accepted")
{
    const FirmwareUpdateModel model;
    FirmwareUpdateController controller{model};
    const std::optional<Request> request = shown_request(controller);
    const AcceptUploadsRequest* accept = request ? std::get_if<AcceptUploadsRequest>(&*request) : nullptr;
    CHECK((accept != nullptr && accept->m_accept));
}

TEST_CASE("left, the screen refuses uploads again")
{
    const FirmwareUpdateModel model;
    FirmwareUpdateController controller{model};
    const std::optional<Request> request = left_request(controller);
    const AcceptUploadsRequest* accept = request ? std::get_if<AcceptUploadsRequest>(&*request) : nullptr;
    CHECK((accept != nullptr && !accept->m_accept));
}

TEST_CASE("ok plays or stops the radio, the knob sets its volume, while the screen waits for an image")
{
    const FirmwareUpdateModel model;
    FirmwareUpdateController controller{model};
    const Response ok = handle_command(controller, OkCommand{});
    const PlayerRequest* toggle = ok.m_request ? std::get_if<PlayerRequest>(&*ok.m_request) : nullptr;
    CHECK((toggle != nullptr && toggle->m_toggle_playing));
    const Response turn = handle_command(controller, DecrementCommand{.m_steps = 2});
    const PlayerRequest* volume = turn.m_request ? std::get_if<PlayerRequest>(&*turn.m_request) : nullptr;
    CHECK((volume != nullptr && volume->m_volume_change == -2 * PlaybackControls::volume_step));
}

TEST_CASE("the screen can be left while waiting and after a failure")
{
    FirmwareUpdateModel model;
    const FirmwareUpdateController controller{model};
    CHECK(may_leave(controller));
    model.m_state = FirmwareUpdateState::failed;
    CHECK(may_leave(controller));
}

TEST_CASE("the screen can't be left while an image is written or the radio restarts")
{
    FirmwareUpdateModel model;
    const FirmwareUpdateController controller{model};
    for (const FirmwareUpdateState state : {FirmwareUpdateState::receiving, FirmwareUpdateState::restarting}) {
        model.m_state = state;
        CHECK_FALSE(may_leave(controller));
    }
}

}  // namespace
}  // namespace avela
