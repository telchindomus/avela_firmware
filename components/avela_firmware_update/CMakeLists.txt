idf_component_register(
    SRCS "src/service/esp_firmware_update_service.cpp"
    INCLUDE_DIRS "include"
    REQUIRES avela_messaging avela_strings avela_ui avela_network esp_http_server freertos
    PRIV_REQUIRES avela_setup app_update esp_app_format esp_system esp_timer log)
