set(firmware "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(avela_hal "${firmware}/components/avela_hal")
idf_component_register(
    SRCS "button_state_machine_test.cpp" "input_poller_test.cpp"
         "${avela_hal}/src/button/button_state_machine.cpp"
    INCLUDE_DIRS "${avela_hal}/include"
    WHOLE_ARCHIVE)
target_link_libraries(${COMPONENT_LIB} PRIVATE Catch2WithMain)
