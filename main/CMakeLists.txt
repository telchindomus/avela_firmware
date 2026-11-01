# The chosen skin is not in REQUIRES: the top-level CMakeLists.txt links it in (main doesn't know outside skins).
idf_component_register(
    SRCS "main.cpp" "application.cpp"
    INCLUDE_DIRS "."
    REQUIRES avela_strings avela_hal avela_boards avela_ui avela_clock avela_settings avela_setup avela_network
             avela_firmware_update avela_radio avela_system_info avela_status_tray avela_weather esp_timer heap freertos
             log)
