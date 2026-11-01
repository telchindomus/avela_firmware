#include "avela_hal/display/st7789_display.hpp"

namespace avela {

//--------------------------------------------------------------------------------------------------

St7789Display::St7789Display(const St7789Config& display_config)
{
    const auto& pins = display_config.m_pins;
    {
        auto config = m_bus_.config();
        config.spi_host = display_config.m_bus.m_host;
        config.spi_mode = 0;
        config.freq_write = display_config.m_bus.m_write_hz;
        config.freq_read = display_config.m_bus.m_read_hz;
        config.spi_3wire = false;
        config.use_lock = true;
        config.dma_channel = SPI_DMA_CH_AUTO;
        config.pin_sclk = pins.m_sclk;
        config.pin_mosi = pins.m_mosi;
        config.pin_miso = -1;
        config.pin_dc = pins.m_data_command;
        m_bus_.config(config);
        m_panel_.setBus(&m_bus_);
    }
    {
        auto config = m_panel_.config();
        config.pin_cs = pins.m_chip_select;
        config.pin_rst = pins.m_reset;
        config.pin_busy = -1;
        const auto& panel = display_config.m_panel;
        config.memory_width = panel.m_memory_width;
        config.memory_height = panel.m_memory_height;
        config.panel_width = panel.m_width;
        config.panel_height = panel.m_height;
        config.offset_x = panel.m_offset_x;
        config.offset_y = 0;
        config.offset_rotation = 0;
        config.readable = false;  // no MISO
        config.invert = panel.m_invert;
        config.rgb_order = false;
        config.dlen_16bit = false;
        config.bus_shared = false;
        m_panel_.config(config);
    }
    {
        auto config = m_backlight_.config();
        config.pin_bl = pins.m_backlight;
        config.invert = false;
        config.freq = display_config.m_backlight.m_pwm_hz;
        config.pwm_channel = display_config.m_backlight.m_pwm_channel;
        m_backlight_.config(config);
        m_panel_.setLight(&m_backlight_);
    }
    setPanel(&m_panel_);
}

}  // namespace avela
