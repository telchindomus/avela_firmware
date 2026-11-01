# Strict warnings for our code (main and components/avela_*); every warning is an error. All other components'
# headers (ESP-IDF, FreeRTOS, LovyanGFX, Catch2) are marked as system headers so their own warnings stay out of our
# build. Used by the firmware and by every test project (call after project()).
set(avela_strict_warnings
    -Wall -Wextra -Werror
    -Wconversion -Wsign-conversion -Wdouble-promotion
    -Wshadow -Wold-style-cast -Wcast-align -Wcast-qual
    -Wnon-virtual-dtor -Woverloaded-virtual
    -Wnull-dereference -Wformat=2 -Wimplicit-fallthrough -Wmisleading-indentation)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # GCC only; clang (the clang-tidy configuration in build_clang/) doesn't know these
    list(APPEND avela_strict_warnings -Wuseless-cast -Wduplicated-cond -Wduplicated-branches -Wlogical-op)
endif()

function(avela_apply_strict_warnings)
    idf_build_get_property(build_components BUILD_COMPONENTS)
    foreach(component IN LISTS build_components)
        idf_component_get_property(component_library ${component} COMPONENT_LIB)
        get_target_property(library_type ${component_library} TYPE)
        if(component STREQUAL "main" OR component MATCHES "^avela_")
            # header-only components (board descriptions) have nothing to compile; their headers get the warnings
            # where included
            if(NOT library_type STREQUAL "INTERFACE_LIBRARY")
                target_compile_options(${component_library} PRIVATE ${avela_strict_warnings})
            endif()
        else()
            set_target_properties(${component_library} PROPERTIES SYSTEM TRUE)
        endif()
    endforeach()
    # Catch2 (the unit tests) comes in as its own CMake target, not as a component
    foreach(library IN ITEMS Catch2 Catch2WithMain)
        if(TARGET ${library})
            set_target_properties(${library} PROPERTIES SYSTEM TRUE)
        endif()
    endforeach()
endfunction()
