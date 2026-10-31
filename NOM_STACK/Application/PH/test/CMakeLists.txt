cmake_minimum_required(VERSION 3.16)
project(test_ph_application LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

get_filename_component(AERO_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)
include("${AERO_REPO_ROOT}/test_support/cmake/AeroGTestCommon.cmake")

aero_add_gtest(test_ph_application
    SOURCES
        test_ph_application.cpp
        ../src/ph_application.c
        ../../WL/src/wl_application.c
        ../../../Services/PUMP_IO_COM/src/pump_io_com.c
        ../../../Signals/Sensors/src/sensor_ic_signals.c
)

