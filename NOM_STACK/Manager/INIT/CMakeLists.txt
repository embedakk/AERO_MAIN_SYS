set(initialization_srcs
    src/initialization.c)

set(initialization_inc_dir 
    inc)

idf_component_register(
    SRCS ${initialization_srcs} 
    INCLUDE_DIRS ${initialization_inc_dir}
    PRIV_REQUIRES   PUMP_IO_COM I2C_COM PH_sensor EC_sensor WL_sensor WTEMP_sensor 
                    RealTimeClock Sensors DWIN_protocol) 