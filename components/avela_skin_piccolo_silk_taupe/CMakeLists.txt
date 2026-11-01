set(skin_assets "${CMAKE_CURRENT_LIST_DIR}/assets/generated/${CONFIG_AVELA_SCREEN_NAME}")
# keep in sync with FONT_ROLES in tools/generate_assets.py
set(font_roles display headline title heading figure subtitle list_item body body_strong caption badge)

set(skin_sources "")
set(embedded_files "")
set(include_directories "include")
if(CONFIG_AVELA_SKIN_PICCOLO_SILK_TAUPE)
    list(APPEND skin_sources
        "src/fonts.cpp"
        "src/widgets.cpp"
        "src/header_skin.cpp"
        "src/tiles_skin.cpp"
        "src/player_skin.cpp"
        "src/stations_skin.cpp"
        "src/info_skin.cpp"
        "src/wifi_setup_skin.cpp"
        "src/firmware_update_skin.cpp")
    if(CONFIG_AVELA_WEATHER)
        list(APPEND skin_sources "src/weather_skin.cpp" "src/weather_icons.cpp")
    endif()
    list(APPEND embedded_files "${skin_assets}/avela_logo.argb")
    foreach(role IN LISTS font_roles)
        list(APPEND embedded_files "${skin_assets}/font_${role}.vlw")
    endforeach()
    list(APPEND include_directories "${skin_assets}")  # only when chosen: the board's screen may have no assets here
endif()

# REQUIRES are unconditional: ESP-IDF resolves them before menuconfig exists (docs/developer/SKIN_ARCHITECTURE.md, rule
# 2)
idf_component_register(
    SRCS ${skin_sources}
    INCLUDE_DIRS ${include_directories}
    EMBED_FILES ${embedded_files}
    REQUIRES avela_strings avela_hal avela_ui avela_clock avela_network avela_radio avela_system_info avela_status_tray
             avela_weather avela_firmware_update
             lovyan03__lovyangfx)
