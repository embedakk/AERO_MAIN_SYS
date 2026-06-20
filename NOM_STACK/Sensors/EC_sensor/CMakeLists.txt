set(ec_sensor_srcs
    src/ec_sensor.c)

set(ec_sensor_inc_dir 
    inc)

idf_component_register(
    SRCS ${ec_sensor_srcs} 
    INCLUDE_DIRS ${ec_sensor_inc_dir}
    PRIV_REQUIRES I2C_COM)