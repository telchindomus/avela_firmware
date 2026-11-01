// What every skin of the Silk Taupe skin draws with: the canvas and the skin's fonts. main creates both once; each skin
// gets the surface when the Ui builds its screen.
#pragma once

#include "avela_skin_piccolo_silk_taupe/fonts.hpp"
#include "avela_ui/canvas.hpp"

namespace avela::skin_piccolo_silk_taupe {

struct Surface {
    Canvas& m_canvas;
    const Fonts& m_fonts;
};

}  // namespace avela::skin_piccolo_silk_taupe
