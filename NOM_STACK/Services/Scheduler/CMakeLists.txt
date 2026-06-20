set(scheduler_srcs
    src/scheduler.c
    src/scheduler_tasks.c)

set(scheduler_inc_dir 
    inc)

idf_component_register(
    SRCS ${scheduler_srcs} 
    INCLUDE_DIRS ${scheduler_inc_dir}
    PRIV_REQUIRES   INIT NormalOp PH_sensor EC_sensor RealTimeClock WL_sensor WTEMP_sensor Sensors 
                    WL EC PH LED Pipes DWIN_protocol MAIN_SYS) 