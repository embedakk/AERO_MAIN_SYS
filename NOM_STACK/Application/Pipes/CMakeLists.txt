set(pipe_watering_app_srcs
    src/pipe_water_ctrl_app.c)

set(pipe_watering_app_inc_dir 
    inc)

idf_component_register(
    SRCS ${pipe_watering_app_srcs} 
    INCLUDE_DIRS ${pipe_watering_app_inc_dir}
    PRIV_REQUIRES   PUMP_IO_COM I2C_COM PH_sensor EC_sensor WL_sensor WTEMP_sensor RealTimeClock Sensors)