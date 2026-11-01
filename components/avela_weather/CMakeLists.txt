# The weather sources compile only when CONFIG_AVELA_WEATHER is on; REQUIRES stay unconditional (ESP-IDF
# resolves them before Kconfig).
set(weather_sources "")
if(CONFIG_AVELA_WEATHER)
    list(APPEND weather_sources
        "src/service/open_meteo.cpp"
        "src/service/open_meteo_weather_service.cpp"
        "src/setup/weather_section.cpp")
endif()

idf_component_register(
    SRCS ${weather_sources}
    INCLUDE_DIRS "include"
    REQUIRES avela_messaging avela_strings avela_ui avela_setup avela_network freertos
    PRIV_REQUIRES avela_hal avela_settings esp_http_client esp-tls mbedtls esp_timer log)
