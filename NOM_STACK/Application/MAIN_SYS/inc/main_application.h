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
    float   ec_of_water[3];
    float   temp_of_room[3];
    float   hum_of_room[3];
}hs_assign_fruit_params_to_process;



static const hs_assign_fruit_params_to_process strawberry =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {5.9, 6.1, 6.2},
    .ec_of_water          =   {2.0, 2.30, 2.70},
    .temp_of_room         =   {20.00, 21.00, 23.00},
    .hum_of_room          =   {70.00, 60.00, 42.00},
};

static const hs_assign_fruit_params_to_process lettuce =
{
    .period_of_lights     =   {14, 16, 17},
    .ph_of_water          =   {6.1, 6.20, 6.30},
    .ec_of_water          =   {1.0, 1.1, 1.2},
    .temp_of_room         =   {20.00, 20.00, 20.00},
    .hum_of_room          =   {70.00, 65.00, 58.00},
};

static const hs_assign_fruit_params_to_process leak =
{
    .period_of_lights     =   {14, 16, 17},
    .ph_of_water          =   {6.1, 6.20, 6.30},
    .ec_of_water          =   {1.0, 1.1, 1.2},
    .temp_of_room         =   {20.00, 20.00, 20.00},
    .hum_of_room          =   {70.00, 65.00, 58.00},
};

static const hs_assign_fruit_params_to_process brussel_sprouts =
{
    .period_of_lights     =   {14, 16, 17},
    .ph_of_water          =   {6.1, 6.20, 6.30},
    .ec_of_water          =   {1.0, 1.1, 1.2},
    .temp_of_room         =   {20.00, 20.00, 20.00},
    .hum_of_room          =   {70.00, 65.00, 58.00},
};

static const hs_assign_fruit_params_to_process tomatoes =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.0, 6.0, 6.0},
    .ec_of_water          =   {2.20, 2.20, 2.20},
    .temp_of_room         =   {21.00, 25.00, 35.00},
    .hum_of_room          =   {62.00, 62.00, 62.00},
};

static const hs_assign_fruit_params_to_process cucumber =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20, 6.50, 7.50},
    .ec_of_water          =   {2.20, 2.20, 2.20},
    .temp_of_room         =   {21.00, 25.00, 35.00},
    .hum_of_room          =   {62.00, 62.00, 62.00},
};

static const hs_assign_fruit_params_to_process cabbage =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20, 6.50, 7.50},
    .ec_of_water          =   {2.20, 2.20, 2.20},
    .temp_of_room         =   {21.00, 25.00, 35.00},
    .hum_of_room          =   {62.00, 62.00, 62.00},
};

static const hs_assign_fruit_params_to_process kale =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20, 6.50, 7.50},
    .ec_of_water          =   {2.20, 2.20, 2.20},
    .temp_of_room         =   {21.00, 25.00, 35.00},
    .hum_of_room          =   {62.00, 62.00, 62.00},
};

static const hs_assign_fruit_params_to_process eggplant =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20, 6.50, 7.50},
    .ec_of_water          =   {2.20, 2.20, 2.20},
    .temp_of_room         =   {21.00, 25.00, 35.00},
    .hum_of_room          =   {62.00, 62.00, 62.00},
};

static const hs_assign_fruit_params_to_process pepper =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20, 6.50, 7.50},
    .ec_of_water          =   {2.20, 2.20, 2.20},
    .temp_of_room         =   {21.00, 25.00, 35.00},
    .hum_of_room          =   {62.00, 62.00, 62.00},
};

static const hs_assign_fruit_params_to_process chilli =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20, 6.50, 7.50},
    .ec_of_water          =   {2.20, 2.20, 2.20},
    .temp_of_room         =   {21.00, 25.00, 35.00},
    .hum_of_room          =   {62.00, 62.00, 62.00},
};

static const hs_assign_fruit_params_to_process peas =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20, 6.50, 7.50},
    .ec_of_water          =   {2.20, 2.20, 2.20},
    .temp_of_room         =   {21.00, 25.00, 35.00},
    .hum_of_room          =   {62.00, 62.00, 62.00},
};

static const hs_assign_fruit_params_to_process aubergine = // changed to arugola
{
    .period_of_lights     =   {14, 17, 19},
    .ph_of_water          =   {6.20, 6.80, 7.20},
    .ec_of_water          =   {1.90, 2.10, 2.30},
    .temp_of_room         =   {18.0, 19.00, 21.00},
    .hum_of_room          =   {70.00, 65.00, 58.00},
};

static const hs_assign_fruit_params_to_process rocket = // changed to arugola
{
    .period_of_lights     =   {14, 15, 15},
    .ph_of_water          =   {6.20, 6.5, 7.00},
    .ec_of_water          =   {1.30, 1.40, 1.50},
    .temp_of_room         =   {18.0, 19.00, 21.00},
    .hum_of_room          =   {70.00, 65.00, 58.00},
};

static const hs_assign_fruit_params_to_process beans =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20, 6.50, 7.50},
    .ec_of_water          =   {2.20, 2.20, 2.20},
    .temp_of_room         =   {21.00, 25.00, 35.00},
    .hum_of_room          =   {62.00, 62.00, 62.00},
};

static const hs_assign_fruit_params_to_process grean_beans =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20, 6.50, 7.50},
    .ec_of_water          =   {2.20, 2.20, 2.20},
    .temp_of_room         =   {21.00, 25.00, 35.00},
    .hum_of_room          =   {62.00, 62.00, 62.00},
};

static const hs_assign_fruit_params_to_process broccoli =
{
    .period_of_lights     =   {14, 17, 20},
    .ph_of_water          =   {6.20, 6.50, 7.50},
    .ec_of_water          =   {2.20, 2.20, 2.20},
    .temp_of_room         =   {21.00, 25.00, 35.00},
    .hum_of_room          =   {62.00, 62.00, 62.00},
};

static const hs_assign_fruit_params_to_process spinach =
{
    .period_of_lights     =   {8, 9, 10},
    .ph_of_water          =   {6.20, 6.50, 7.00},
    .ec_of_water          =   {1.80, 2.10, 2.30},
    .temp_of_room         =   {18.00, 20.00, 20.00},
    .hum_of_room          =   {70.00, 65.00, 58.00},
};

static const hs_assign_fruit_params_to_process asian_greans =
{
    .period_of_lights     =   {8, 9, 10},
    .ph_of_water          =   {6.20, 6.50, 7.00},
    .ec_of_water          =   {1.80, 2.10, 2.30},
    .temp_of_room         =   {18.00, 20.00, 20.00},
    .hum_of_room          =   {70.00, 65.00, 58.00},
};

static const hs_assign_fruit_params_to_process parsley =
{
    .period_of_lights     =   {8, 9, 10},
    .ph_of_water          =   {6.20, 6.80, 7.20},
    .ec_of_water          =   {1.90, 2.10, 2.30},
    .temp_of_room         =   {18.0, 19.00, 21.00},
    .hum_of_room          =   {70.00, 65.00, 58.00},
};

static const hs_assign_fruit_params_to_process fruit_sel_null =
{
    .period_of_lights     =   {0, 0, 0},
    .ph_of_water          =   {0, 0, 0},
    .ec_of_water          =   {0, 0, 0},
    .temp_of_room         =   {0, 0, 0},
    .hum_of_room          =   {0, 0, 0},
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

#endif //MAIN_APPLICATION_H