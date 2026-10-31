cmake_minimum_required(VERSION 3.16)
project(test_real_time_clock LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

get_filename_component(AERO_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)
include("${AERO_REPO_ROOT}/test_support/cmake/AeroGTestCommon.cmake")

aero_add_gtest(test_real_time_clock
    SOURCES
        test_real_time_clock.cpp
        ../src/real_time_clock.c
        ../../../Services/I2C_COM/src/i2c_com.c
)

