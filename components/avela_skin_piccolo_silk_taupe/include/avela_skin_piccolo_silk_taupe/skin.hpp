// The Silk Taupe skin's entry header: Avela on the 1.9" screen in mushroom taupe, espresso and champagne gold (Manrope,
// the Avela symbol with a gold sun). Every skin has `<its component>/skin.hpp` naming `Skin`: its page skin for every
// screen (each fulfils that screen's skin concept), the fonts it loads, the surface its skins draw on and the screen
// its layouts are for. main/skin.hpp includes the chosen skin's (menuconfig: Avela skin;
// docs/developer/SKIN_ARCHITECTURE.md).
#pragma once

#include "avela_skin_piccolo_silk_taupe/firmware_update_skin.hpp"
#include "avela_skin_piccolo_silk_taupe/fonts.hpp"
#include "avela_skin_piccolo_silk_taupe/header_skin.hpp"
#include "avela_skin_piccolo_silk_taupe/info_skin.hpp"
#include "avela_skin_piccolo_silk_taupe/player_skin.hpp"
#include "avela_skin_piccolo_silk_taupe/screen.hpp"
#include "avela_skin_piccolo_silk_taupe/stations_skin.hpp"
#include "avela_skin_piccolo_silk_taupe/surface.hpp"
#include "avela_skin_piccolo_silk_taupe/tiles_skin.hpp"
#include "avela_skin_piccolo_silk_taupe/wifi_setup_skin.hpp"
#include "sdkconfig.h"
#if CONFIG_AVELA_WEATHER
#include "avela_skin_piccolo_silk_taupe/weather_skin.hpp"
#endif

namespace avela {

struct PiccoloSilkTaupeSkin {
    using Fonts = skin_piccolo_silk_taupe::Fonts;
    using Surface = skin_piccolo_silk_taupe::Surface;

    using HeaderSkin = skin_piccolo_silk_taupe::HeaderSkin;
    using TilesSkin = skin_piccolo_silk_taupe::TilesSkin;
    using PlayerSkin = skin_piccolo_silk_taupe::PlayerSkin;
    using StationsSkin = skin_piccolo_silk_taupe::StationsSkin;
    using InfoSkin = skin_piccolo_silk_taupe::InfoSkin;
    using WifiSetupSkin = skin_piccolo_silk_taupe::WifiSetupSkin;
    using FirmwareUpdateSkin = skin_piccolo_silk_taupe::FirmwareUpdateSkin;
#if CONFIG_AVELA_WEATHER
    using WeatherSkin = skin_piccolo_silk_taupe::WeatherSkin;
#endif

    static constexpr ScreenSize screen = skin_piccolo_silk_taupe::screen;  // what its layouts of this build are for
};

using Skin = PiccoloSilkTaupeSkin;

}  // namespace avela
