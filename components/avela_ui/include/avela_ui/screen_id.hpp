// All screens of the app, by name. Lives here, not in main, because a controller names the screen it opens, also one
// in another component, and the skins' tiles name the screens they show. The screens of optional components exist only
// when the component is on (menuconfig), like the rest of the component. Adding a screen: one line here; Ui::build()
// and every skin's tiles then fail to build until they handle it.
#pragma once

#include <cstdint>

#include "sdkconfig.h"

namespace avela {

enum class ScreenId : uint8_t {
    tiles,
    player,
    stations,
    info,
    wifi_setup,
    firmware_update,
#if CONFIG_AVELA_WEATHER
    weather,
#endif
};

}  // namespace avela
