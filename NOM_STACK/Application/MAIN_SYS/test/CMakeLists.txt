cmake_minimum_required(VERSION 3.16)
project(test_main_sys LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

get_filename_component(AERO_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)
include("${AERO_REPO_ROOT}/test_support/cmake/AeroGTestCommon.cmake")

aero_add_gtest(test_main_sys
    SOURCES
        test_main_sys.cpp
        ../src/main_application.c
        ../../../Signals/Sensors/src/sensor_ic_signals.c
)

