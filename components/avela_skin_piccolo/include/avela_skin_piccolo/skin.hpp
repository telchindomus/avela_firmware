// The piccolo skin's entry header: Avela's own look on the 1.9" screen (deep teal, the TelchinDomus palette, Manrope,
// the Avela symbol). Every skin has `<its component>/skin.hpp` naming `Skin`: its page skin for
// every screen (each fulfils that screen's skin concept), the fonts it loads, the surface its skins draw on and the
// screen its layouts are for. main/skin.hpp includes the chosen skin's (menuconfig: Avela skin;
// docs/developer/SKIN_ARCHITECTURE.md).
#pragma once

#include "avela_skin_piccolo/firmware_update_skin.hpp"
#include "avela_skin_piccolo/fonts.hpp"
#include "avela_skin_piccolo/header_skin.hpp"
#include "avela_skin_piccolo/info_skin.hpp"
#include "avela_skin_piccolo/player_skin.hpp"
#include "avela_skin_piccolo/screen.hpp"
#include "avela_skin_piccolo/stations_skin.hpp"
#include "avela_skin_piccolo/surface.hpp"
#include "avela_skin_piccolo/tiles_skin.hpp"
#include "avela_skin_piccolo/wifi_setup_skin.hpp"
#include "sdkconfig.h"
#if CONFIG_AVELA_WEATHER
#include "avela_skin_piccolo/weather_skin.hpp"
#endif

namespace avela {

struct PiccoloSkin {
    using Fonts = skin_piccolo::Fonts;
    using Surface = skin_piccolo::Surface;

    using HeaderSkin = skin_piccolo::HeaderSkin;
    using TilesSkin = skin_piccolo::TilesSkin;
    using PlayerSkin = skin_piccolo::PlayerSkin;
    using StationsSkin = skin_piccolo::StationsSkin;
    using InfoSkin = skin_piccolo::InfoSkin;
    using WifiSetupSkin = skin_piccolo::WifiSetupSkin;
    using FirmwareUpdateSkin = skin_piccolo::FirmwareUpdateSkin;
#if CONFIG_AVELA_WEATHER
    using WeatherSkin = skin_piccolo::WeatherSkin;
#endif

    static constexpr ScreenSize screen = skin_piccolo::screen;  // what its layouts of this build are for
};

using Skin = PiccoloSkin;

}  // namespace avela
