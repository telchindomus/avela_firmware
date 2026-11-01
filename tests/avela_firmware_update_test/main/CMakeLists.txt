set(firmware "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(avela_firmware_update "${firmware}/components/avela_firmware_update")
set(avela_network "${firmware}/components/avela_network")
set(avela_ui "${firmware}/components/avela_ui")
set(avela_messaging "${firmware}/components/avela_messaging")
set(avela_strings "${firmware}/components/avela_strings")
idf_component_register(
    SRCS "firmware_update_view_test.cpp" "firmware_update_controller_test.cpp"
    INCLUDE_DIRS "${avela_firmware_update}/include" "${avela_network}/include" "${avela_ui}/include"
                 "${avela_messaging}/include"
                 "${avela_strings}/include"
    REQUIRES freertos
    WHOLE_ARCHIVE)
target_link_libraries(${COMPONENT_LIB} PRIVATE Catch2WithMain)
