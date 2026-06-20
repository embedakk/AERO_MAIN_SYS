set(ph_sensor_srcs
    src/ph_sensor.c)

set(ph_sensor_inc_dir 
    inc)

idf_component_register(
    SRCS ${ph_sensor_srcs} 
    INCLUDE_DIRS ${ph_sensor_inc_dir}
    PRIV_REQUIRES I2C_COM)