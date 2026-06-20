set(wl_sensor_srcs
    src/wl_sensor.c)

set(wl_sensor_inc_dir 
    inc)

idf_component_register(
    SRCS ${wl_sensor_srcs} 
    INCLUDE_DIRS ${wl_sensor_inc_dir})