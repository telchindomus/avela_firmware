// Holds the header (the status tray's view over the skin's header skin) and the shown screen below it: the
// components' screens over the chosen skin's page skins, built from the models (UiModels) when shown.
// The screens form a tree by how they open each other: a controller opens its children (Response::m_show), and the
// Ui keeps the path from the tiles (the root) to the shown screen. Back closes the shown screen, back to the one that
// opened it (nothing on the tiles), so does a controller's Response::m_close; the other commands go to the shown
// screen's controller (and only to it). The requests the controllers make go to `route`, given to start(), handle()
// and refresh(): a callable taking a `const Request&` (the application's: into the queue of the service it is for).
// Every frame the shown screen's view and the header refresh.
// App rule: when the setup network closes while the Wi-Fi setup screen is shown (a network was saved on the setup
// page), that screen closes, back to Info, where the joining shows.
#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <variant>

#include "avela_firmware_update/ui/firmware_update_screen.hpp"
#include "avela_messaging/messages/request.hpp"
#include "avela_network/ui/wifi_setup_screen.hpp"
#include "avela_radio/ui/player_screen.hpp"
#include "avela_radio/ui/stations_screen.hpp"
#include "avela_status_tray/status_tray_view.hpp"
#include "avela_system_info/ui/info_screen.hpp"
#include "avela_ui/command/command.hpp"
#include "avela_ui/controller_calls.hpp"
#include "avela_ui/screen_concept.hpp"
#include "avela_ui/screen_id.hpp"
#include "avela_ui/tiles/tiles_screen.hpp"
#include "router_concept.hpp"
#include "sdkconfig.h"
#include "ui_models.hpp"
#if CONFIG_AVELA_WEATHER
#include "avela_weather/ui/weather_screen.hpp"
#endif

// The chosen skin's entry header (`<skin component>/skin.hpp`, naming `Skin`): the top-level CMakeLists.txt checks the
// skin (menuconfig: Avela skin), links it into main and sets AVELA_SKIN_HEADER. Each view template checks its page
// skin against the component's concept.
#include AVELA_SKIN_HEADER

// The chosen board's header (menuconfig: Avela board), which names `Board`.
#include CONFIG_AVELA_BOARD_HEADER

namespace avela {

class Ui {
public:
    // Starts on the player, one level under the tiles.
    Ui(const Skin::Surface& surface, UiModels models)
        : m_surface_(surface), m_models_(models),
          m_header_(models.m_radio, models.m_network, Skin::HeaderSkin{surface}),
          m_screen_(std::in_place_type<Player>, models.m_radio, Skin::PlayerSkin{surface})
    {}

    // Once: draws the header and the first screen whole.
    template <RouterConcept RouterType>
    void start(RouterType& route)
    {
        draw(route);
    }

    // Back closes the shown screen. Any other command goes to the shown screen's controller: its request to
    // `route`, then the screen it opens or closes.
    template <RouterConcept RouterType>
    void handle(const Command& command, RouterType& route)
    {
        if (std::holds_alternative<BackCommand>(command)) {
            close(route);
            return;
        }
        const Response next = std::visit(
            [&](auto& screen) {
                const Response response = handle_command(screen.m_controller, command);
                if (response.m_request) {
                    route(*response.m_request);
                }
                return response;
            },
            m_screen_);
        if (next.m_close) {
            close(route);
        } else if (next.m_show) {
            open(*next.m_show, route);
        }
    }

    template <RouterConcept RouterType>
    void refresh(RouterType& route)
    {
        if (m_models_.m_network.m_setup_network_active != m_shown_setup_network_active_) {
            m_shown_setup_network_active_ = m_models_.m_network.m_setup_network_active;
            if (!m_shown_setup_network_active_ && current() == ScreenId::wifi_setup) {
                close(route);
                return;
            }
        }
        m_header_.refresh();
        std::visit([](auto& screen) { screen.m_view.refresh(); }, m_screen_);
    }

private:
    using Tiles = TilesScreen<Skin::TilesSkin>;
    using Player = PlayerScreen<Skin::PlayerSkin>;
    using Stations = StationsScreen<Skin::StationsSkin>;
    using Info = InfoScreen<Skin::InfoSkin>;
    using WifiSetup = WifiSetupScreen<Skin::WifiSetupSkin>;
    using FirmwareUpdate = FirmwareUpdateScreen<Skin::FirmwareUpdateSkin>;
#if CONFIG_AVELA_WEATHER
    using Weather = WeatherScreen<Skin::WeatherSkin>;
    using ScreenVariant = std::variant<Tiles, Player, Stations, Info, WifiSetup, FirmwareUpdate, Weather>;
    static_assert(ScreenConcept<Weather>);
#else
    using ScreenVariant = std::variant<Tiles, Player, Stations, Info, WifiSetup, FirmwareUpdate>;
#endif

    static_assert(ScreenConcept<Tiles> && ScreenConcept<Player> && ScreenConcept<Stations> && ScreenConcept<Info> &&
                  ScreenConcept<WifiSetup> && ScreenConcept<FirmwareUpdate>);

    // The screens the tiles offer (their children), in this order (an optional component's only when it is on).
    static constexpr std::array tile_screens{
        ScreenId::player,
#if CONFIG_AVELA_WEATHER
        ScreenId::weather,
#endif
        ScreenId::info,
    };

    // The deepest path: tiles -> Info -> firmware update, and one spare level.
    static constexpr std::size_t path_capacity = 4;

    static_assert(Skin::screen.m_width == Board::screen.m_width && Skin::screen.m_height == Board::screen.m_height,
                  "the skin's layouts are not for the board's screen");

    // A child of the shown screen: one level down.
    template <RouterConcept RouterType>
    void open(ScreenId id, RouterType& route)
    {
        if (m_depth_ == path_capacity || !leave(route)) {
            return;
        }
        const ScreenId from = current();
        m_path_.at(m_depth_++) = id;
        build(id, from);
        draw(route);
    }

    // Back to the screen that opened the shown one: one level up; nothing on the tiles.
    template <RouterConcept RouterType>
    void close(RouterType& route)
    {
        if (m_depth_ == 1 || !leave(route)) {
            return;
        }
        const ScreenId closed = current();
        --m_depth_;
        build(current(), closed);
        draw(route);
    }

    // The shown screen's controller lets it go (false: it can't be left now) and makes its request for being left.
    template <RouterConcept RouterType>
    bool leave(RouterType& route)
    {
        return std::visit(
            [&route](auto& screen) {
                if (!may_leave(screen.m_controller)) {
                    return false;
                }
                if (const auto request = left_request(screen.m_controller)) {
                    route(*request);
                }
                return true;
            },
            m_screen_);
    }

    // from: the screen the Ui comes from (the tiles open on its branch)
    void build(ScreenId id, ScreenId from)
    {
        switch (id) {
        case ScreenId::tiles:
            m_screen_.emplace<Tiles>(tile_screens, from, Skin::TilesSkin{m_surface_});
            break;
        case ScreenId::player:
            m_screen_.emplace<Player>(m_models_.m_radio, Skin::PlayerSkin{m_surface_});
            break;
        case ScreenId::stations:
            m_screen_.emplace<Stations>(m_models_.m_radio, Skin::StationsSkin{m_surface_});
            break;
        case ScreenId::info:
            m_screen_.emplace<Info>(m_models_.m_system_info, m_models_.m_network, Skin::InfoSkin{m_surface_});
            break;
        case ScreenId::wifi_setup:
            m_screen_.emplace<WifiSetup>(m_models_.m_network, Skin::WifiSetupSkin{m_surface_});
            break;
        case ScreenId::firmware_update:
            m_screen_.emplace<FirmwareUpdate>(m_models_.m_firmware_update, m_models_.m_network,
                                              Skin::FirmwareUpdateSkin{m_surface_});
            break;
#if CONFIG_AVELA_WEATHER
        case ScreenId::weather:
            m_screen_.emplace<Weather>(m_models_.m_weather, m_models_.m_network, Skin::WeatherSkin{m_surface_});
            break;
#endif
        }
    }

    // The header and the shown screen whole; its controller's request for being shown, if any, goes to `route`.
    template <RouterConcept RouterType>
    void draw(RouterType& route)
    {
        m_header_.draw();
        std::visit(
            [&route](auto& screen) {
                screen.m_view.draw();
                if (const auto request = shown_request(screen.m_controller)) {
                    route(*request);
                }
            },
            m_screen_);
    }

    [[nodiscard]] ScreenId current() const { return m_path_.at(m_depth_ - 1); }

    Skin::Surface m_surface_;
    UiModels m_models_;
    StatusTrayView<Skin::HeaderSkin> m_header_;
    ScreenVariant m_screen_;  // the last screen on the path
    std::array<ScreenId, path_capacity> m_path_{ScreenId::tiles, ScreenId::player};
    std::size_t m_depth_ = 2;
    bool m_shown_setup_network_active_ = false;
};

}  // namespace avela
