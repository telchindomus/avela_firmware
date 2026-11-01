#include "avela_ui/tiles/tiles_screen.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <span>

#include "avela_ui/screen_concept.hpp"

namespace avela {
namespace {

// Keeps the chosen tile it was handed last.
class FakeSkin {
public:
    explicit FakeSkin(std::size_t& selected) : m_selected_(&selected) {}

    void set_tiles(std::span<const ScreenId> /*screens*/, std::size_t selected) { *m_selected_ = selected; }
    void draw() {}

private:
    std::size_t* m_selected_;
};

static_assert(TilesSkinConcept<FakeSkin>);
static_assert(ScreenConcept<TilesScreen<FakeSkin>>);

constexpr std::array screens{ScreenId::player, ScreenId::stations, ScreenId::info};

TEST_CASE("the tiles open on the screen just closed")
{
    std::size_t selected = 99;
    TilesScreen screen{screens, ScreenId::info, FakeSkin{selected}};
    screen.m_view.draw();
    CHECK(selected == 2);
}

TEST_CASE("the tiles open on the first tile when the closed screen isn't one of theirs")
{
    std::size_t selected = 99;
    TilesScreen screen{screens, ScreenId::tiles, FakeSkin{selected}};
    screen.m_view.draw();
    CHECK(selected == 0);
}

}  // namespace
}  // namespace avela
