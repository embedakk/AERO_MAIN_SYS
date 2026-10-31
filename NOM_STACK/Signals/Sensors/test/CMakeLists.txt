cmake_minimum_required(VERSION 3.16)
project(test_sensor_ic_signals LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

get_filename_component(AERO_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)
include("${AERO_REPO_ROOT}/test_support/cmake/AeroGTestCommon.cmake")

aero_add_gtest(test_sensor_ic_signals
    SOURCES
        test_sensor_ic_signals.cpp
        ../src/sensor_ic_signals.c
)

