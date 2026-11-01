idf_component_register(
    SRCS "src/service/esp_system_info_service.cpp"
    INCLUDE_DIRS "include"
    REQUIRES avela_strings avela_ui avela_network
    PRIV_REQUIRES esp_app_format esp_timer heap freertos log)
