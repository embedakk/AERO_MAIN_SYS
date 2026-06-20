set(main_sys_srcs
    src/main_application.c)

set(main_sys_app_inc_dir 
    inc)

idf_component_register(
    SRCS ${main_sys_srcs} 
    INCLUDE_DIRS ${main_sys_app_inc_dir}
    PRIV_REQUIRES   RealTimeClock Sensors) 