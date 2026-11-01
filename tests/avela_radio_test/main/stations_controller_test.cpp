#include "avela_radio/ui/stations_controller.hpp"

#include <catch2/catch_test_macros.hpp>
#include <variant>

namespace avela {
namespace {

TEST_CASE("increment and decrement move the selection in the page model, wrapping around")
{
    const RadioModel model;  // the default stations
    StationsModel page{.m_selected = 0};
    StationsController controller{model, page};
    const auto count = model.m_stations.size();
    controller.handle(DecrementCommand{.m_steps = 1});
    CHECK(page.m_selected == count - 1);
    controller.handle(IncrementCommand{.m_steps = 2});
    CHECK(page.m_selected == 1);
}

TEST_CASE("ok asks to play the selected station by its slot and closes the list")
{
    const RadioModel model;
    StationsModel page{.m_selected = 2};
    const StationsController controller{model, page};
    const Response response = controller.handle(OkCommand{});
    CHECK(response.m_close);
    CHECK_FALSE(response.m_show.has_value());
    const PlayerRequest* sent = response.m_request ? std::get_if<PlayerRequest>(&*response.m_request) : nullptr;
    const PlayerRequest request = sent != nullptr ? *sent : PlayerRequest{};
    CHECK(request.m_station_slot == model.m_stations.slot(2));
    CHECK(request.m_playing == true);
    CHECK(model.m_station_index == 0);
}

TEST_CASE("a list that shrank while open keeps the selection on a station it still has")
{
    RadioModel model;
    StationsModel page{.m_selected = 5};
    StationsController controller{model, page};
    model.m_stations.clear();  // stations removed on the setup page: one left
    REQUIRE(model.m_stations.add(9, "Left", "Jazz"));
    CHECK(page.selected(model.m_stations.size()) == 0);

    const Response response = controller.handle(OkCommand{});
    const PlayerRequest* sent = response.m_request ? std::get_if<PlayerRequest>(&*response.m_request) : nullptr;
    REQUIRE(sent != nullptr);
    CHECK(sent->m_station_slot == 9);

    controller.handle(IncrementCommand{.m_steps = 1});
    CHECK(page.m_selected == 0);
}

}  // namespace
}  // namespace avela
