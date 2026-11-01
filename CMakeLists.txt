cmake_minimum_required(VERSION 3.16)

# The skin API versions this firmware accepts: a pip-style specifier, comma-separated clauses that must all hold
# (>=, >, <=, <, ==, !=). Its lower bound moves to the new version in the commit that makes skins add or change
# something, with an entry in docs/developer/SKIN_CHANGES.md (docs/developer/SKIN_ARCHITECTURE.md: Versions and skin
# compatibility).
set(supported_skin_api_versions ">=1.0.0")

# The firmware version: X.Y.Z on a release (tag vX.Y.Z), X.Y.Z-dev on devel between releases, X.Y.Z being the next
# release. It is what the firmware reports (Info, the update page, the User-Agent); the skin checks below compare its
# number. cicd/release.sh sets it, tools/check_skin_api.sh checks its form (docs/developer/SKIN_ARCHITECTURE.md:
# Versions and skin compatibility).
set(PROJECT_VER "1.0.1-dev")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(avela_firmware)

include(cmake/strict_warnings.cmake)
avela_apply_strict_warnings()

# The chosen skin (menuconfig: Avela skin). main can't name it in its REQUIRES (it doesn't know outside skins), so it is
# checked and linked into main here; main/ui.hpp includes its entry header (docs/developer/SKIN_ARCHITECTURE.md,
# rules 3-5).
string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" firmware_version "${PROJECT_VER}")
set(skin "${CONFIG_AVELA_SKIN_COMPONENT}")
set(skin_api_version "${CONFIG_AVELA_SKIN_API_VERSION}")
if(skin_api_version VERSION_GREATER firmware_version)
    message(FATAL_ERROR "skin ${skin} implements skin API ${skin_api_version}, newer than this firmware "
                        "(${firmware_version}): update the firmware.")
endif()
string(REPLACE "," ";" skin_api_clauses "${supported_skin_api_versions}")
foreach(clause IN LISTS skin_api_clauses)
    string(STRIP "${clause}" clause)
    if(NOT clause MATCHES "^(>=|<=|==|!=|>|<) *([0-9]+(\\.[0-9]+)*)$")
        message(FATAL_ERROR "supported_skin_api_versions: can't read \"${clause}\"")
    endif()
    set(operator "${CMAKE_MATCH_1}")
    set(version "${CMAKE_MATCH_2}")
    # the skin's API version compared with the clause's: -1 older, 0 equal, 1 newer; each operator allows some of them
    if(skin_api_version VERSION_LESS version)
        set(order -1)
    elseif(skin_api_version VERSION_EQUAL version)
        set(order 0)
    else()
        set(order 1)
    endif()
    if(operator STREQUAL ">=")
        set(allowed_orders 0 1)
    elseif(operator STREQUAL ">")
        set(allowed_orders 1)
    elseif(operator STREQUAL "<=")
        set(allowed_orders -1 0)
    elseif(operator STREQUAL "<")
        set(allowed_orders -1)
    elseif(operator STREQUAL "==")
        set(allowed_orders 0)
    else()  # !=
        set(allowed_orders -1 1)
    endif()
    if(NOT order IN_LIST allowed_orders)
        message(FATAL_ERROR "skin ${skin} implements skin API ${skin_api_version}; this firmware (${firmware_version}) "
                            "supports ${supported_skin_api_versions}. See docs/developer/SKIN_CHANGES.md for what the skin has to "
                            "add or change.")
    endif()
endforeach()

idf_component_get_property(main_library main COMPONENT_LIB)
idf_component_get_property(skin_library ${skin} COMPONENT_LIB)
target_link_libraries(${main_library} PRIVATE ${skin_library})
target_compile_definitions(${main_library} PRIVATE AVELA_SKIN_HEADER="${skin}/skin.hpp")
