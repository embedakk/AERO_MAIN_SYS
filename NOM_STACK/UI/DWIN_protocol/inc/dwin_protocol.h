#ifndef DWIN_PROTOCOL_H
#define DWIN_PROTOCOL_H

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"

#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"


#define UART_NUM                                                UART_NUM_1
#define BUF_SIZE                                                1024

#define TX_PIN                                                  27
#define RX_PIN                                                  33


#define         DWIN_PROTOCOL_TAG                               "DWIN_PROTOCOL"

#define         DWIN_PROTOCOL_LOGS
#undef          DWIN_PROTOCOL_LOGS

//WRITE COMMANDS MCU TO DISPLAY
#define         UI_DWIN_WRITE_ADDR                              0x82

#define         UI_DWIN_WRITE_VIEW_PH_ADDRESS                   0x5000
#define         UI_DWIN_WRITE_VIEW_EC_ADDRESS                   0x5010
#define         UI_DWIN_WRITE_VIEW_W_TEMP_ADDRESS               0x5020
#define         UI_DWIN_WRITE_VIEW_DISTANCE_ADDRESS             0x5030
#define         UI_DWIN_WRITE_VIEW_TANK_CAP_ADDRESS             0x5040
#define         UI_DWIN_WRITE_VIEW_R_TEMP_ADDRESS               0x5050
#define         UI_DWIN_WRITE_VIEW_HUMIDITY_ADDRESS             0x5060
#define         UI_DWIN_WRITE_VIEW_FRUIT_ADDRESS                0x5070

#define         UI_DWIN_WRITE_VIEW_FRUIT_TXT_ADDRESS            0x1000
#define         UI_DWIN_WRITE_VIEW_LED_STATUS_TXT_ADDRESS       0x1020
#define         UI_DWIN_WRITE_VIEW_PUMP_STATUS_TXT_ADDRESS      0x1040
#define         UI_DWIN_WRITE_VIEW_SYS_STATUS_TXT_ADDRESS       0x1060


#define         UI_DWIN_WRITE_CONFIG_TANK_CAP_ADDRESS           0x5200              
#define         UI_DWIN_WRITE_CONFIG_DOSING_INT_ADDRESS         0x5210              
#define         UI_DWIN_WRITE_CONFIG_PH_SOLUTION_ADDRESS        0x5220              
#define         UI_DWIN_WRITE_CONFIG_EC_SOLUTION_ADDRESS        0x5230              
#define         UI_DWIN_WRITE_CONFIG_MIXING_TIME_ADDRESS        0x5240   

#define         UI_DWIN_WRITE_RTC_YEAR_ADDRESS                  0x5300              
#define         UI_DWIN_WRITE_RTC_MONTH_ADDRESS                 0x5310
#define         UI_DWIN_WRITE_RTC_TIME_ADDRESS                  0x5320
#define         UI_DWIN_WRITE_RTC_HOUR_ADDRESS                  0x5330
#define         UI_DWIN_WRITE_RTC_MINUTE_ADDRESS                0x5340

#define         UI_DWIN_WRITE_SETTING_EC_V_ADDRESS              0x5350
#define         UI_DWIN_WRITE_SETTING_PH_V_ADDRESS              0x5390

#define         UI_DWIN_WRITE_EDIT_SYS_STAGE_ADDRESS            0x53D0

//READ COMMANDS DISPLAY TO MCU
#define         UI_DWIN_READ_SETTING_SYS_START_ADDRESS          0x5100
#define         UI_DWIN_READ_SETTING_SYS_CONTINUE_ADDRESS       0x5110
#define         UI_DWIN_READ_SETTING_SYS_DELETE_ADDRESS         0x5120
#define         UI_DWIN_READ_SETTING_SYS_STOP_ADDRESS           0x5130
#define         UI_DWIN_READ_SETTING_FILL_TANK_ADDRESS          0x5140
#define         UI_DWIN_READ_SETTING_RELEASE_TANK_ADDRESS       0x5150


#define         UI_DWIN_READ_CONFIG_TANK_CAP_ADDRESS            0x5200//2b              
#define         UI_DWIN_READ_CONFIG_DOSING_INT_ADDRESS          0x5210//2b          
#define         UI_DWIN_READ_CONFIG_PH_SOLUTION_ADDRESS         0x5220//4b              
#define         UI_DWIN_READ_CONFIG_EC_SOLUTION_ADDRESS         0x5230//4b              
#define         UI_DWIN_READ_CONFIG_MIXING_TIME_ADDRESS         0x5240//2b      

#define         UI_DWIN_READ_RTC_YEAR_ADDRESS                   0x5300              
#define         UI_DWIN_READ_RTC_MONTH_ADDRESS                  0x5310
#define         UI_DWIN_READ_RTC_TIME_ADDRESS                   0x5320
#define         UI_DWIN_READ_RTC_HOUR_ADDRESS                   0x5330
#define         UI_DWIN_READ_RTC_MINUTE_ADDRESS                 0x5340

#define         UI_DWIN_READ_SETTING_EC_CAL_L_ADDRESS           0x5360
#define         UI_DWIN_READ_SETTING_EC_CAL_M_ADDRESS           0x5370
#define         UI_DWIN_READ_SETTING_EC_CAL_H_ADDRESS           0x5380

#define         UI_DWIN_READ_SETTING_PH_CAL_L_ADDRESS           0x53A0
#define         UI_DWIN_READ_SETTING_PH_CAL_M_ADDRESS           0x53B0
#define         UI_DWIN_READ_SETTING_PH_CAL_H_ADDRESS           0x53C0

#define         UI_DWIN_READ_EDIT_SYS_STAGE_ADDRESS             0x53D0

#define         UI_DWIN_READ_EDIT_BROCCOLI_ADDRESS              0x5500
#define         UI_DWIN_READ_EDIT_STRAWBERRY_ADDRESS            0x5510
#define         UI_DWIN_READ_EDIT_ROCKET_ADDRESS                0x5520
#define         UI_DWIN_READ_EDIT_TOMATO_ADDRESS                0x5530
#define         UI_DWIN_READ_EDIT_LETTUCE_ADDRESS               0x5540
#define         UI_DWIN_READ_EDIT_LEAK_ADDRESS                  0x5550
#define         UI_DWIN_READ_EDIT_EGGPLANT_ADDRESS              0x5560
#define         UI_DWIN_READ_EDIT_PEAS_ADDRESS                  0x5570
#define         UI_DWIN_READ_EDIT_BR_SPROUTS_ADDRESS            0x5580
#define         UI_DWIN_READ_EDIT_CABBAGE_ADDRESS               0x5590
#define         UI_DWIN_READ_EDIT_CHILLI_ADDRESS                0x55A0
#define         UI_DWIN_READ_EDIT_SPINACH_ADDRESS               0x55B0
#define         UI_DWIN_READ_EDIT_PARSLEY_ADDRESS               0x55C0
#define         UI_DWIN_READ_EDIT_GREEN_BEANS_ADDRESS           0x55D0
#define         UI_DWIN_READ_EDIT_CUCUMBER_ADDRESS              0x55E0
#define         UI_DWIN_READ_EDIT_KALE_ADDRESS                  0x55F0
#define         UI_DWIN_READ_EDIT_ASIAN_GREANS_ADDRESS          0x55F8

typedef enum
{
    UI_BROCCOLI_ADDRESS        = 0,
    UI_STRAWBERRY_ADDRESS      = 1,
    UI_ROCKET_ADDRESS          = 2,
    UI_TOMATO_ADDRESS          = 3,
    UI_LETTUCE_ADDRESS         = 4,
    UI_LEAK_ADDRESS            = 5,
    UI_EGGPLANT_ADDRESS        = 6,
    UI_PEAS_ADDRESS            = 7,
    UI_BR_SPROUTS_ADDRESS      = 8,
    UI_CABBAGE_ADDRESS         = 9,
    UI_CHILLI_ADDRESS          = 10,
    UI_SPINACH_ADDRESS         = 11,
    UI_PARSLEY_ADDRESS         = 12,
    UI_GREEN_BEANS_ADDRESS     = 13,        
    UI_CUCUMBER_ADDRESS        = 14,            
    UI_KALE_ADDRESS            = 15,        
    UI_ASIAN_GREANS_ADDRESS    = 16,  
    UI_WRONG_FRUIT             = 17,
    UI_NO_FRUIT_SELECTED       = 18      
}UI_fruits;

uint8_t ph_conversion_bytes[4];
uint8_t ec_conversion_bytes[4];
uint8_t water_temp_conversion_bytes[4];
uint8_t distance_conversion_bytes[4];
uint8_t tank_cap_percentage_conversion_bytes[4];
uint8_t room_temp_conversion_bytes[4];
uint8_t humidity_conversion_bytes[4];
uint8_t ph_voltage_conversion_bytes[4];
uint8_t ec_voltage_conversion_bytes[4];

void IRAM_ATTR ui_protocol_uart_isr_handler(void *arg);
void ui_protocol_rx_task(void *arg);

void floatToHex(float value, uint8_t *hexBytes); 
void dataToDisplayBuff(uint8_t pointed_addr, float input_data, uint8_t floatBytes[4]);
void createDwinTextCommand(char *text, uint8_t *command);
void check_address(uint8_t *received_data, uint16_t address, uint8_t format);
void parse_received_byte(size_t length_of_data, uint8_t *received_data);

void UI_INIT(void);
void UI_DATA_OUT(void);
void UI_EXE(void);

//events from display to mcu READ COMMAND ADDRESS 0x83
// {5A}{A5}{06}{83}{52}{00}{01}{05}{DC} response from tank capacity 1500 
// {5A}{A5}{06}{83}{52}{10}{01}{00}{02} dosing intensity 2
// {5A}{A5}{08}{83}{52}{20}{02}{00}{00}{AF}{C8}  ph sol 45000
// {5A}{A5}{08}{83}{52}{30}{02}{00}{01}{86}{A0}  ec sol 100000
// {5A}{A5}{06}{83}{52}{40}{01}{00}{FA} mixing time 250


// {5A}{A5}{06}{83}{53}{D0}{01}{00}{02} system stage 2
// {5A}{A5}{06}{83}{55}{E0}{01}{00}{00} fruit cucumber

//RESPONSE AFTER write operation 0x83
// {5A}{A5}{03}{82}{4F}{4B}
//{5A}{A5}{03}{82}{4F}{4B}|{5A}{A5}{06}{83}{53}{D0}{01}{00}{02}
//{5A}{A5}{06}{83}{53}{D0}{01}{00}{02}|{5A}{A5}{03}{82}{4F}{4B} //11
//{5A}{A5}{08}{83}{52}{30}{02}{00}{01}{86}{A0}|{5A}{A5}{03}{82}{4F}{4B} //13  ec sol 100000

//4 formats for 9 byte data
//0 => 7 8
//6 => 13 14
//9 => 16 17
//{5A}{A5}{06}{83}{53}{D0}{01}{00}{02}|{5A}{A5}{06}{83}{53}{D0}{01}{00}{02}
//11 18 19


#endif //DWIN_PROTOCOL_h