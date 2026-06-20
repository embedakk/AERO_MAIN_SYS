set(pump_io_com_srcs
    src/pump_io_com.c)

set(pump_io_com_inc_dir 
    inc)

idf_component_register(
    SRCS ${pump_io_com_srcs} 
    INCLUDE_DIRS ${pump_io_com_inc_dir}) 