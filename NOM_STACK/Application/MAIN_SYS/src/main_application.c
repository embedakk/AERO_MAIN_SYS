#include "../inc/main_application.h"

void MAIN_FRUIT_EXE(void)
{
    switch (signal_data_set_selected_fruit)
    {
        case BROCCOLI_ADDRESS:
            {
                signal_data_get_turn_on_time = broccoli.period_of_lights[1];
                signal_data_get_target_ph_data = broccoli.ph_of_water[1];
                signal_data_get_target_ec_data = broccoli.ec_of_water[1];
            }
            break;

        case STRAWBERRY_ADDRESS:
            {
                signal_data_get_turn_on_time = strawberry.period_of_lights[1];
                signal_data_get_target_ph_data = strawberry.ph_of_water[1];
                signal_data_get_target_ec_data = strawberry.ec_of_water[1];
            }
            break;
        
        case ROCKET_ADDRESS:
            {
                signal_data_get_turn_on_time = rocket.period_of_lights[1];
                signal_data_get_target_ph_data = rocket.ph_of_water[1];
                signal_data_get_target_ec_data = rocket.ec_of_water[1];
            }
            break;

        case TOMATO_ADDRESS:
            {
                signal_data_get_turn_on_time = tomatoes.period_of_lights[1];
                signal_data_get_target_ph_data = tomatoes.ph_of_water[1];
                signal_data_get_target_ec_data = tomatoes.ec_of_water[1];
            }
            break;

        case LETTUCE_ADDRESS:
            {
                signal_data_get_turn_on_time = lettuce.period_of_lights[1];
                signal_data_get_target_ph_data = lettuce.ph_of_water[1];
                signal_data_get_target_ec_data = lettuce.ec_of_water[1];
            }
            break;
        
        case LEAK_ADDRESS:
            {
                signal_data_get_turn_on_time = leak.period_of_lights[1];
                signal_data_get_target_ph_data = leak.ph_of_water[1];
                signal_data_get_target_ec_data = leak.ec_of_water[1];
            }
            break;

        case EGGPLANT_ADDRESS:
            {
                signal_data_get_turn_on_time = eggplant.period_of_lights[1];
                signal_data_get_target_ph_data = eggplant.ph_of_water[1];
                signal_data_get_target_ec_data = eggplant.ec_of_water[1];
            }
            break;

        case PEAS_ADDRESS:
            {
                signal_data_get_turn_on_time = peas.period_of_lights[1];
                signal_data_get_target_ph_data = peas.ph_of_water[1];
                signal_data_get_target_ec_data = peas.ec_of_water[1];
            }
            break;
        
        case BR_SPROUTS_ADDRESS:
            {
                signal_data_get_turn_on_time = brussel_sprouts.period_of_lights[1];
                signal_data_get_target_ph_data = brussel_sprouts.ph_of_water[1];
                signal_data_get_target_ec_data = brussel_sprouts.ec_of_water[1]; 
            }
            break;

        case CABBAGE_ADDRESS:
            {
                signal_data_get_turn_on_time = cabbage.period_of_lights[1];
                signal_data_get_target_ph_data = cabbage.ph_of_water[1];
                signal_data_get_target_ec_data = cabbage.ec_of_water[1]; 
            }
            break;

        case CHILLI_ADDRESS:
            {
                signal_data_get_turn_on_time = chilli.period_of_lights[1];
                signal_data_get_target_ph_data = chilli.ph_of_water[1];
                signal_data_get_target_ec_data = chilli.ec_of_water[1]; 
            }
            break;
        
        case SPINACH_ADDRESS:
            {
                signal_data_get_turn_on_time = spinach.period_of_lights[1];
                signal_data_get_target_ph_data = spinach.ph_of_water[1];
                signal_data_get_target_ec_data = spinach.ec_of_water[1];
            }
            break;

        case PARSLEY_ADDRESS:
            {
                signal_data_get_turn_on_time = parsley.period_of_lights[1];
                signal_data_get_target_ph_data = parsley.ph_of_water[1];
                signal_data_get_target_ec_data = parsley.ec_of_water[1];
            }
            break;

        case GREEN_BEANS_ADDRESS:
            {
                signal_data_get_turn_on_time = grean_beans.period_of_lights[1];
                signal_data_get_target_ph_data = grean_beans.ph_of_water[1];
                signal_data_get_target_ec_data = grean_beans.ec_of_water[1];
            }
            break;
        
        case CUCUMBER_ADDRESS:
            {
                signal_data_get_turn_on_time = cucumber.period_of_lights[1];
                signal_data_get_target_ph_data = cucumber.ph_of_water[1];
                signal_data_get_target_ec_data = cucumber.ec_of_water[1];
            }
            break;

        case KALE_ADDRESS:
            {
                signal_data_get_turn_on_time = kale.period_of_lights[1];
                signal_data_get_target_ph_data = kale.ph_of_water[1];
                signal_data_get_target_ec_data = kale.ec_of_water[1];
            }
            break;

        case ASIAN_GREANS_ADDRESS:
            {
                signal_data_get_turn_on_time = asian_greans.period_of_lights[1];
                signal_data_get_target_ph_data = asian_greans.ph_of_water[1];
                signal_data_get_target_ec_data = asian_greans.ec_of_water[1];
            }
            break;
        
        case WRONG_FRUIT:
            {
                signal_data_get_turn_on_time = fruit_sel_null.period_of_lights[1];
                signal_data_get_target_ph_data = fruit_sel_null.ph_of_water[1];
                signal_data_get_target_ec_data = fruit_sel_null.ec_of_water[1];
            }
            break;

        case NO_FRUIT_SELECTED:
            {
                signal_data_get_turn_on_time = fruit_sel_null.period_of_lights[1];
                signal_data_get_target_ph_data = fruit_sel_null.ph_of_water[1];
                signal_data_get_target_ec_data = fruit_sel_null.ec_of_water[1];
            }
            break;

            
        default:
            break;
    }
    return;
}