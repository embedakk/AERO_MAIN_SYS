set(normal_op_srcs
    src/normal_op.c)

set(normal_op_inc_dir 
    inc)

idf_component_register(
    SRCS ${normal_op_srcs} 
    INCLUDE_DIRS ${normal_op_inc_dir}
    PRIV_REQUIRES   PUMP_IO_COM I2C_COM PH_sensor EC_sensor WL_sensor WTEMP_sensor RealTimeClock Sensors WL) 