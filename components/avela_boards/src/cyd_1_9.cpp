#include "avela_boards/cyd_1_9.hpp"

namespace avela {

//--------------------------------------------------------------------------------------------------

void Cyd_1_9::init()
{
    m_display_.init();
    m_boot_button_.init();
}

}  // namespace avela
