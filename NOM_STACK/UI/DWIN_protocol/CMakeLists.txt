set(ui_srcs
    src/dwin_protocol.c)

set(ui_inc_dir 
    inc)

idf_component_register(
    SRCS ${ui_srcs} 
    INCLUDE_DIRS ${ui_inc_dir}
    PRIV_REQUIRES RealTimeClock Sensors)