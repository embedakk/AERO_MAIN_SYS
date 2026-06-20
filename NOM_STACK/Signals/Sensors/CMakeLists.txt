set(sensor_ic_signals_srcs
    src/sensor_ic_signals.c)

set(sensor_ic_signals_inc_dir 
    inc)

idf_component_register(
    SRCS ${sensor_ic_signals_srcs} 
    INCLUDE_DIRS ${sensor_ic_signals_inc_dir}
    PRIV_REQUIRES RealTimeClock nvs_flash)