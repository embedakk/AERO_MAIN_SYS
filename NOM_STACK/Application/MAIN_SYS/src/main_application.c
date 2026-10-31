#include "../inc/main_application.h"

void MAIN_FRUIT_EXE(void)
{
    uint8_t stage = (signal_data_set_system_stage_data <= THIRD_PHASE)
                        ? signal_data_set_system_stage_data
                        : THIRD_PHASE;
    signal_data_get_system_stage_data = stage;

    const hs_assign_fruit_params_to_process *profile = &fruit_sel_null;

    switch (signal_data_set_selected_fruit)
    {
        case BROCCOLI_ADDRESS:      profile = &broccoli;        break;
        case STRAWBERRY_ADDRESS:    profile = &strawberry;      break;
        case ROCKET_ADDRESS:        profile = &rocket;          break;
        case TOMATO_ADDRESS:        profile = &tomatoes;        break;
        case LETTUCE_ADDRESS:       profile = &lettuce;         break;
        case LEAK_ADDRESS:          profile = &leak;            break;
        case EGGPLANT_ADDRESS:      profile = &eggplant;        break;
        case PEAS_ADDRESS:          profile = &peas;            break;
        case BR_SPROUTS_ADDRESS:    profile = &brussel_sprouts; break;
        case CABBAGE_ADDRESS:       profile = &cabbage;         break;
        case CHILLI_ADDRESS:        profile = &chilli;          break;
        case SPINACH_ADDRESS:       profile = &spinach;         break;
        case PARSLEY_ADDRESS:       profile = &parsley;         break;
        case GREEN_BEANS_ADDRESS:   profile = &grean_beans;     break;
        case CUCUMBER_ADDRESS:      profile = &cucumber;        break;
        case KALE_ADDRESS:          profile = &kale;            break;
        case ASIAN_GREANS_ADDRESS:  profile = &asian_greans;    break;
        case WRONG_FRUIT:
        case NO_FRUIT_SELECTED:
        default:
            profile = &fruit_sel_null;
            break;
    }

    signal_data_get_turn_on_time   = profile->period_of_lights[stage];
    signal_data_get_target_ph_data = profile->ph_of_water[stage];
    signal_data_get_target_ec_data = profile->ec_of_water[stage];
}
