idf_component_register(
    SRCS
        "src/model/host_name.cpp"
        "src/service/captive_dns.cpp"
        "src/service/esp_wifi_service.cpp"
        "src/setup/wifi_section.cpp"
    INCLUDE_DIRS "include"
    REQUIRES avela_messaging avela_strings avela_ui avela_setup freertos esp_wifi esp_netif esp_event
    PRIV_REQUIRES avela_clock avela_settings esp_timer log lwip)
