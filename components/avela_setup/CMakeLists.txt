idf_component_register(
    SRCS "src/html_writer.cpp" "src/form_fields.cpp" "src/setup_server.cpp" "src/value_writer.cpp"
         "src/serial_console.cpp" "src/setup_console.cpp" "src/apply_lock.cpp" "src/page_password.cpp"
         "src/access_section.cpp" "src/setup_network_client.cpp"
    INCLUDE_DIRS "include"
    REQUIRES avela_strings esp_http_server freertos
    PRIV_REQUIRES avela_settings esp_wifi esp_netif esp_hw_support lwip mbedtls esp_timer esp_driver_uart esp_system log)
