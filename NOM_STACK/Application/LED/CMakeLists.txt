set(led_ctrl_app_srcs
    src/led_ctrl_application.c)

set(led_ctrl_app_inc_dir 
    inc)

idf_component_register(
    SRCS ${led_ctrl_app_srcs} 
    INCLUDE_DIRS ${led_ctrl_app_inc_dir}
    PRIV_REQUIRES   PUMP_IO_COM I2C_COM PH_sensor EC_sensor WL_sensor WTEMP_sensor RealTimeClock Sensors) 