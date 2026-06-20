set(WTEMP_sensor_srcs
    src/wtemp_sensor.c)

set(WTEMP_sensor_inc_dir 
    inc)

idf_component_register(
    SRCS ${WTEMP_sensor_srcs} 
    INCLUDE_DIRS ${WTEMP_sensor_inc_dir})