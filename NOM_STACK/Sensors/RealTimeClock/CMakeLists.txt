set(rtc_srcs
    src/real_time_clock.c)

set(rtc_inc_dir 
    inc)

idf_component_register(
    SRCS ${rtc_srcs} 
    INCLUDE_DIRS ${rtc_inc_dir}
    PRIV_REQUIRES I2C_COM)