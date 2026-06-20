set(i2c_com_srcs
    src/i2c_com.c)

set(i2c_com_inc_dir 
    inc)

idf_component_register(
    SRCS ${i2c_com_srcs} 
    INCLUDE_DIRS ${i2c_com_inc_dir})
    # PRIV_REQUIRES PUMP_IO_COM) 