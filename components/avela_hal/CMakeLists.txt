idf_component_register(
    SRCS "src/display/st7789_display.cpp" "src/button/button_state_machine.cpp" "src/button/gpio_button.cpp"
         "src/encoder/pcnt_encoder.cpp" "src/audio/i2s_output.cpp" "src/memory/heap_watch.cpp"
    INCLUDE_DIRS "include"
    REQUIRES lovyan03__lovyangfx esp_driver_gpio esp_driver_pcnt esp_driver_i2s
    PRIV_REQUIRES heap esp_timer log)
