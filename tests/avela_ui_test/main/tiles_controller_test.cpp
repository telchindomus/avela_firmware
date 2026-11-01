#include "avela_ui/tiles/tiles_controller.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>

#include "avela_ui/controller_calls.hpp"

namespace avela {
namespace {

constexpr std::array screens{ScreenId::player, ScreenId::stations, ScreenId::info};

TEST_CASE("increment and decrement choose a tile in the page model, without wrapping")
{
    TilesModel page;
    TilesController controller{screens, page};
    controller.handle(IncrementCommand{.m_steps = 1});
    CHECK(page.m_selected == 1);
    controller.handle(IncrementCommand{.m_steps = 5});
    CHECK(page.m_selected == 2);
    controller.handle(DecrementCommand{.m_steps = 9});
    CHECK(page.m_selected == 0);
}

TEST_CASE("ok shows the chosen screen")
{
    TilesModel page{.m_selected = 1};
    TilesController controller{screens, page};
    CHECK(handle_command(controller, OkCommand{}).m_show == ScreenId::stations);
}

TEST_CASE("a command the tiles have no meaning for does nothing")
{
    TilesModel page;
    TilesController controller{screens, page};
    CHECK_FALSE(handle_command(controller, MenuCommand{}).m_show.has_value());
    CHECK(page.m_selected == 0);
}

}  // namespace
}  // namespace avela
