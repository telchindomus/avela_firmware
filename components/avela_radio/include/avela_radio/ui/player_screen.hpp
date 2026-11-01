// Now playing: the home screen, the player's view and controller.
#pragma once

#include <utility>

#include "avela_radio/model/radio_model.hpp"
#include "avela_radio/ui/player_controller.hpp"
#include "avela_radio/ui/player_skin_concept.hpp"
#include "avela_radio/ui/player_view.hpp"
#include "avela_ui/screen_id.hpp"

namespace avela {

template <PlayerSkinConcept PageSkin>
struct PlayerScreen {
    static constexpr ScreenId id = ScreenId::player;

    PlayerScreen(const RadioModel& model, PageSkin skin) : m_view{model, std::move(skin)} {}

    PlayerView<PageSkin> m_view;
    PlayerController m_controller;
};

}  // namespace avela
