set(wl_app_srcs
    src/wl_application.c)

set(wl_app_inc_dir 
    inc)

idf_component_register(
    SRCS ${wl_app_srcs} 
    INCLUDE_DIRS ${wl_app_inc_dir}
    PRIV_REQUIRES   PUMP_IO_COM I2C_COM PH_sensor EC_sensor WL_sensor WTEMP_sensor RealTimeClock Sensors) 