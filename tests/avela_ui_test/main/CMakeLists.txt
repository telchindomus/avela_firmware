set(firmware "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(avela_ui "${firmware}/components/avela_ui")
set(avela_messaging "${firmware}/components/avela_messaging")
idf_component_register(
    SRCS "playback_controls_test.cpp" "tiles_controller_test.cpp" "tiles_screen_test.cpp"
    INCLUDE_DIRS "${avela_ui}/include" "${avela_messaging}/include"
    WHOLE_ARCHIVE)
target_link_libraries(${COMPONENT_LIB} PRIVATE Catch2WithMain)
