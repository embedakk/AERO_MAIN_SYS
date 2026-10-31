cmake_minimum_required(VERSION 3.16)
project(test_i2c_com LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

get_filename_component(AERO_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)
include("${AERO_REPO_ROOT}/test_support/cmake/AeroGTestCommon.cmake")

aero_add_gtest(test_i2c_com
    SOURCES
        test_i2c_com.cpp
        ../src/i2c_com.c
)

