#include "avela_boards/avela_piccolo.hpp"

namespace avela {

//--------------------------------------------------------------------------------------------------

void AvelaPiccolo::init()
{
    Cyd_1_9::init();
    display().setRotation(display_rotation);
    m_knob_.init();
    m_knob_button_.init();
    m_amplifier_.init();
}

}  // namespace avela
