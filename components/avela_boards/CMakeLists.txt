set(board_sources "src/cyd_1_9.cpp" "src/setup/board_section.cpp")
if(CONFIG_AVELA_BOARD_PICCOLO)
    list(APPEND board_sources "src/avela_piccolo.cpp")
endif()

# REQUIRES are unconditional: ESP-IDF resolves them before menuconfig exists
idf_component_register(
    SRCS ${board_sources}
    INCLUDE_DIRS "include"
    REQUIRES avela_hal avela_settings avela_setup lovyan03__lovyangfx
    PRIV_REQUIRES esp_system freertos)
