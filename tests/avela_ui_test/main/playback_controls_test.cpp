#include "avela_ui/playback_controls.hpp"

#include <catch2/catch_test_macros.hpp>
#include <variant>

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

TEST_CASE("ok asks to play or stop the other way, whatever the model says")
{
    const Response response = PlaybackControls::handle(OkCommand{});
    CHECK(response.m_request.has_value());
    const PlayerRequest request = player_request_of(response);
    CHECK(request.m_toggle_playing);
    CHECK_FALSE(request.m_playing.has_value());
    CHECK_FALSE(response.m_show.has_value());
    CHECK_FALSE(response.m_close);
}

TEST_CASE("increment and decrement ask for a volume change, a step per detent")
{
    CHECK(player_request_of(PlaybackControls::handle(IncrementCommand{.m_steps = 3})).m_volume_change ==
          3 * PlaybackControls::volume_step);
    CHECK(player_request_of(PlaybackControls::handle(DecrementCommand{.m_steps = 1})).m_volume_change ==
          -PlaybackControls::volume_step);
}

}  // namespace
}  // namespace avela
