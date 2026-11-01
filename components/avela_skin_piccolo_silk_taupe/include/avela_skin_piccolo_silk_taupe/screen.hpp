// The screen this build's layouts are for: the board's (menuconfig: Avela board picks it with the board; main
// checks it against the board's description). The Silk Taupe skin has layouts for these screens; every layout header
// includes layouts/<screen>/<page>.hpp.
#pragma once

#include "avela_hal/board/screen_size.hpp"
#include "sdkconfig.h"

namespace avela::skin_piccolo_silk_taupe {

#if CONFIG_AVELA_SCREEN_LANDSCAPE_320X170
inline constexpr ScreenSize screen{.m_width = 320, .m_height = 170};
#else
#error "the Silk Taupe skin has no layout for this screen"
#endif

}  // namespace avela::skin_piccolo_silk_taupe
