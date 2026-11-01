#include "avela_radio/ui/player_controller.hpp"

#include <catch2/catch_test_macros.hpp>
#include <variant>

#include "avela_ui/controller_calls.hpp"

namespace avela {
namespace {

// The player request a response carries; an empty one if it carries none.
PlayerRequest player_request_of(const Response& response)
{
    if (!response.m_request) {
        return {};
    }
    const PlayerRequest* request = std::get_if<PlayerRequest>(&*response.m_request);
    return request != nullptr ? *request : PlayerRequest{};
}

TEST_CASE("the player takes the playback controls: ok plays or stops, the knob sets the volume")
{
    PlayerController controller;
    CHECK(player_request_of(handle_command(controller, OkCommand{})).m_toggle_playing);
    CHECK(player_request_of(handle_command(controller, IncrementCommand{.m_steps = 1})).m_volume_change ==
          PlaybackControls::volume_step);
}

TEST_CASE("menu opens the station list")
{
    PlayerController controller;
    const Response response = handle_command(controller, MenuCommand{});
    CHECK(response.m_show == ScreenId::stations);
    CHECK_FALSE(response.m_request.has_value());
}

TEST_CASE("a command the player has no meaning for does nothing")
{
    PlayerController controller;
    const Response response = handle_command(controller, OptionsCommand{});
    CHECK_FALSE(response.m_show.has_value());
    CHECK_FALSE(response.m_request.has_value());
}

}  // namespace
}  // namespace avela
