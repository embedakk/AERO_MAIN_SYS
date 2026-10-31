cmake_minimum_required(VERSION 3.16)
project(test_init_manager LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

get_filename_component(AERO_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)
include("${AERO_REPO_ROOT}/test_support/cmake/AeroGTestCommon.cmake")

aero_add_gtest(test_init_manager
    SOURCES
        test_init_manager.cpp
        ../src/initialization.c
        ../../../Services/PUMP_IO_COM/src/pump_io_com.c
        ../../../Services/I2C_COM/src/i2c_com.c
        ../../../Sensors/PH_sensor/src/ph_sensor.c
        ../../../Sensors/EC_sensor/src/ec_sensor.c
        ../../../Sensors/WL_sensor/src/wl_sensor.c
        ../../../Sensors/WTEMP_sensor/src/wtemp_sensor.c
        ../../../Sensors/RealTimeClock/src/real_time_clock.c
        ../../../UI/DWIN_protocol/src/dwin_protocol.c
        ../../../Signals/Sensors/src/sensor_ic_signals.c
)

