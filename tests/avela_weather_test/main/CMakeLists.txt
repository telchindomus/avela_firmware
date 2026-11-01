set(firmware "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(avela_weather "${firmware}/components/avela_weather")
set(avela_strings "${firmware}/components/avela_strings")
idf_component_register(
    SRCS "open_meteo_test.cpp" "weather_settings_test.cpp"
         "${avela_weather}/src/service/open_meteo.cpp"
    INCLUDE_DIRS "${avela_weather}/include" "${avela_strings}/include"
    WHOLE_ARCHIVE)
target_link_libraries(${COMPONENT_LIB} PRIVATE Catch2WithMain)
