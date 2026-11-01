cmake_minimum_required(VERSION 3.16)

set(firmware "${CMAKE_CURRENT_LIST_DIR}/../..")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(COMPONENTS main)
# the FreeRTOS mock: no scheduler, and Catch2 provides main()
list(APPEND EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/tools/mocks/freertos/")
project(avela_setup_test)

include(${firmware}/cmake/strict_warnings.cmake)
avela_apply_strict_warnings()
