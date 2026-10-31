#ifndef MAIN_APPLICATION_H
#define MAIN_APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "../../NOM_STACK/Sensors/RealTimeClock/inc/real_time_clock.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"

#ifdef __cplusplus
extern "C" {
#endif

static const uint8_t    FIRST_PHASE     = 0;
static const uint8_t    SECOND_PHASE    = 1;
static const uint8_t    THIRD_PHASE     = 2;

typedef enum
{
    FIRST_PHASE_TIME_PERIOD     =   15,
    SECOND_PHASE_TIME_PERIOD    =   28
}hs_period_timer;

typedef struct 
{
    uint8_t period_of_lights;
    float   ph_of_water;
    float   ec_of_water;
    float   temp_of_room;
    float   hum_of_room;
}hs_auto_process_exe;

typedef struct
{
    uint8_t period_of_lights[3]; // 51 byte one structure for any fruit
    float   ph_of_water[3];
    float   ec_of_water[3];      // uS/cm (matching EC sensor and DWIN display units)
    float   temp_of_room[3];
    float   hum_of_room[3];
}hs_assign_fruit_params_to_process;

static const hs_assign_fruit_params_to_process strawberry =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {5.9f, 6.1f, 6.2f},
    .ec_of_water          =   {2000.0f, 2300.0f, 2700.0f},
    .temp_of_room         =   {20.00f, 21.00f, 23.00f},
    .hum_of_room          =   {70.00f, 60.00f, 42.00f},
};

static const hs_assign_fruit_params_to_process lettuce =
{
    .period_of_lights     =   {14, 16, 17},
    .ph_of_water          =   {6.1f, 6.20f, 6.30f},
    .ec_of_water          =   {1000.0f, 1100.0f, 1200.0f},
    .temp_of_room         =   {20.00f, 20.00f, 20.00f},
    .hum_of_room          =   {70.00f, 65.00f, 58.00f},
};

static const hs_assign_fruit_params_to_process leak =
{
    .period_of_lights     =   {14, 16, 17},
    .ph_of_water          =   {6.1f, 6.20f, 6.30f},
    .ec_of_water          =   {1000.0f, 1100.0f, 1200.0f},
    .temp_of_room         =   {20.00f, 20.00f, 20.00f},
    .hum_of_room          =   {70.00f, 65.00f, 58.00f},
};

static const hs_assign_fruit_params_to_process brussel_sprouts =
{
    .period_of_lights     =   {14, 16, 17},
    .ph_of_water          =   {6.1f, 6.20f, 6.30f},
    .ec_of_water          =   {1000.0f, 1100.0f, 1200.0f},
    .temp_of_room         =   {20.00f, 20.00f, 20.00f},
    .hum_of_room          =   {70.00f, 65.00f, 58.00f},
};

static const hs_assign_fruit_params_to_process tomatoes =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.0f, 6.0f, 6.0f},
    .ec_of_water          =   {2200.0f, 2200.0f, 2200.0f},
    .temp_of_room         =   {21.00f, 25.00f, 35.00f},
    .hum_of_room          =   {62.00f, 62.00f, 62.00f},
};

static const hs_assign_fruit_params_to_process cucumber =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20f, 6.50f, 7.50f},
    .ec_of_water          =   {2200.0f, 2200.0f, 2200.0f},
    .temp_of_room         =   {21.00f, 25.00f, 35.00f},
    .hum_of_room          =   {62.00f, 62.00f, 62.00f},
};

static const hs_assign_fruit_params_to_process cabbage =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20f, 6.50f, 7.50f},
    .ec_of_water          =   {2200.0f, 2200.0f, 2200.0f},
    .temp_of_room         =   {21.00f, 25.00f, 35.00f},
    .hum_of_room          =   {62.00f, 62.00f, 62.00f},
};

static const hs_assign_fruit_params_to_process kale =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20f, 6.50f, 7.50f},
    .ec_of_water          =   {2200.0f, 2200.0f, 2200.0f},
    .temp_of_room         =   {21.00f, 25.00f, 35.00f},
    .hum_of_room          =   {62.00f, 62.00f, 62.00f},
};

static const hs_assign_fruit_params_to_process eggplant =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20f, 6.50f, 7.50f},
    .ec_of_water          =   {2200.0f, 2200.0f, 2200.0f},
    .temp_of_room         =   {21.00f, 25.00f, 35.00f},
    .hum_of_room          =   {62.00f, 62.00f, 62.00f},
};

static const hs_assign_fruit_params_to_process pepper =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20f, 6.50f, 7.50f},
    .ec_of_water          =   {2200.0f, 2200.0f, 2200.0f},
    .temp_of_room         =   {21.00f, 25.00f, 35.00f},
    .hum_of_room          =   {62.00f, 62.00f, 62.00f},
};

static const hs_assign_fruit_params_to_process chilli =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20f, 6.50f, 7.50f},
    .ec_of_water          =   {2200.0f, 2200.0f, 2200.0f},
    .temp_of_room         =   {21.00f, 25.00f, 35.00f},
    .hum_of_room          =   {62.00f, 62.00f, 62.00f},
};

static const hs_assign_fruit_params_to_process peas =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20f, 6.50f, 7.50f},
    .ec_of_water          =   {2200.0f, 2200.0f, 2200.0f},
    .temp_of_room         =   {21.00f, 25.00f, 35.00f},
    .hum_of_room          =   {62.00f, 62.00f, 62.00f},
};

static const hs_assign_fruit_params_to_process aubergine = // changed to arugola
{
    .period_of_lights     =   {14, 17, 19},
    .ph_of_water          =   {6.20f, 6.80f, 7.20f},
    .ec_of_water          =   {1900.0f, 2100.0f, 2300.0f},
    .temp_of_room         =   {18.0f, 19.00f, 21.00f},
    .hum_of_room          =   {70.00f, 65.00f, 58.00f},
};

static const hs_assign_fruit_params_to_process rocket = // changed to arugola
{
    .period_of_lights     =   {14, 15, 15},
    .ph_of_water          =   {6.20f, 6.5f, 7.00f},
    .ec_of_water          =   {1300.0f, 1400.0f, 1500.0f},
    .temp_of_room         =   {18.0f, 19.00f, 21.00f},
    .hum_of_room          =   {70.00f, 65.00f, 58.00f},
};

static const hs_assign_fruit_params_to_process beans =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20f, 6.50f, 7.50f},
    .ec_of_water          =   {2200.0f, 2200.0f, 2200.0f},
    .temp_of_room         =   {21.00f, 25.00f, 35.00f},
    .hum_of_room          =   {62.00f, 62.00f, 62.00f},
};

static const hs_assign_fruit_params_to_process grean_beans =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20f, 6.50f, 7.50f},
    .ec_of_water          =   {2200.0f, 2200.0f, 2200.0f},
    .temp_of_room         =   {21.00f, 25.00f, 35.00f},
    .hum_of_room          =   {62.00f, 62.00f, 62.00f},
};

static const hs_assign_fruit_params_to_process broccoli =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20f, 6.50f, 7.50f},
    .ec_of_water          =   {2200.0f, 2200.0f, 2200.0f},
    .temp_of_room         =   {21.00f, 25.00f, 35.00f},
    .hum_of_room          =   {62.00f, 62.00f, 62.00f},
};

static const hs_assign_fruit_params_to_process spinach =
{
    .period_of_lights     =   {8, 9, 10},
    .ph_of_water          =   {6.20f, 6.50f, 7.00f},
    .ec_of_water          =   {1800.0f, 2100.0f, 2300.0f},
    .temp_of_room         =   {18.00f, 20.00f, 20.00f},
    .hum_of_room          =   {70.00f, 65.00f, 58.00f},
};

static const hs_assign_fruit_params_to_process asian_greans =
{
    .period_of_lights     =   {8, 9, 10},
    .ph_of_water          =   {6.20f, 6.50f, 7.00f},
    .ec_of_water          =   {1800.0f, 2100.0f, 2300.0f},
    .temp_of_room         =   {18.00f, 20.00f, 20.00f},
    .hum_of_room          =   {70.00f, 65.00f, 58.00f},
};

static const hs_assign_fruit_params_to_process parsley =
{
    .period_of_lights     =   {8, 9, 10},
    .ph_of_water          =   {6.20f, 6.80f, 7.20f},
    .ec_of_water          =   {1900.0f, 2100.0f, 2300.0f},
    .temp_of_room         =   {18.0f, 19.00f, 21.00f},
    .hum_of_room          =   {70.00f, 65.00f, 58.00f},
};

static const hs_assign_fruit_params_to_process fruit_sel_null =
{
    .period_of_lights     =   {0, 0, 0},
    .ph_of_water          =   {0.0f, 0.0f, 0.0f},
    .ec_of_water          =   {0.0f, 0.0f, 0.0f},
    .temp_of_room         =   {0.0f, 0.0f, 0.0f},
    .hum_of_room          =   {0.0f, 0.0f, 0.0f},
};

typedef enum
{
    BROCCOLI_ADDRESS        = 0,
    STRAWBERRY_ADDRESS      = 1,
    ROCKET_ADDRESS          = 2,
    TOMATO_ADDRESS          = 3,
    LETTUCE_ADDRESS         = 4,
    LEAK_ADDRESS            = 5,
    EGGPLANT_ADDRESS        = 6,
    PEAS_ADDRESS            = 7,
    BR_SPROUTS_ADDRESS      = 8,
    CABBAGE_ADDRESS         = 9,
    CHILLI_ADDRESS          = 10,
    SPINACH_ADDRESS         = 11,
    PARSLEY_ADDRESS         = 12,
    GREEN_BEANS_ADDRESS     = 13,        
    CUCUMBER_ADDRESS        = 14,            
    KALE_ADDRESS            = 15,        
    ASIAN_GREANS_ADDRESS    = 16,  
    WRONG_FRUIT             = 17,
    NO_FRUIT_SELECTED       = 18      
}UI_fruits_params;

void MAIN_FRUIT_EXE(void);

#ifdef __cplusplus
}
#endif

#endif //MAIN_APPLICATION_H
