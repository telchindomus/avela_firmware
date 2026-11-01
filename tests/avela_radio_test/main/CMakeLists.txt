set(firmware "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(avela_radio "${firmware}/components/avela_radio")
set(avela_ui "${firmware}/components/avela_ui")
set(avela_messaging "${firmware}/components/avela_messaging")
set(avela_strings "${firmware}/components/avela_strings")
idf_component_register(
    SRCS "icy_metadata_reader_test.cpp" "volume_test.cpp" "spectrum_bands_test.cpp" "player_view_test.cpp"
         "player_controller_test.cpp" "stations_controller_test.cpp" "carry_out_test.cpp"
         "station_list_test.cpp" "station_fields_test.cpp"
         "${avela_radio}/src/stream/icy_metadata_reader.cpp" "${avela_radio}/src/audio/volume.cpp"
         "${avela_radio}/src/audio/spectrum_bands.cpp" "${avela_radio}/src/model/station_list.cpp"
         "${avela_radio}/src/setup/station_fields.cpp" "${avela_radio}/src/model/carry_out.cpp"
    INCLUDE_DIRS "${avela_radio}/include" "${avela_ui}/include" "${avela_messaging}/include" "${avela_strings}/include"
    WHOLE_ARCHIVE)
target_link_libraries(${COMPONENT_LIB} PRIVATE Catch2WithMain)
