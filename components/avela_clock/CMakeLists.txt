idf_component_register(
    SRCS "src/system_clock.cpp" "src/setup/clock_section.cpp"
    INCLUDE_DIRS "include"
    REQUIRES avela_strings avela_settings avela_setup)
