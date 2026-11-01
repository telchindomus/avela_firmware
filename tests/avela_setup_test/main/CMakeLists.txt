set(firmware "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(avela_setup "${firmware}/components/avela_setup")
set(avela_strings "${firmware}/components/avela_strings")
idf_component_register(
    SRCS "page_password_test.cpp" "setup_network_client_test.cpp" "${avela_setup}/src/page_password.cpp"
         "${avela_setup}/src/setup_network_client.cpp"
    INCLUDE_DIRS "${avela_setup}/include" "${avela_strings}/include"
    REQUIRES freertos mbedtls
    WHOLE_ARCHIVE)
target_link_libraries(${COMPONENT_LIB} PRIVATE Catch2WithMain)
