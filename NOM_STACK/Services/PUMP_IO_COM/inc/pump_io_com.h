#ifndef PUMP_IO_COM_H
#define PUMP_IO_COM_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/uart.h"

//UART2
#define PUMP_IO_COM_TXD_PIN     (GPIO_NUM_16) 
#define PUMP_IO_COM_RXD_PIN     (GPIO_NUM_17)

#define                         PUMP_IO_COM_TAG         "PUMP_IO_COM"

#define                         PUMP_IO_COM_LOGS
#undef                          PUMP_IO_COM_LOGS

bool busy_flag;


typedef struct 
{
    uint8_t start_byte;
    uint8_t interface_byte;
    uint8_t command_byte;
    uint8_t checksum_byte;
    uint8_t end_byte;
}pump_ctrl_protocol_rcv_buffer;

// typedef struct 
// {
//     uint8_t start_byte;
//     uint8_t interface_byte;
//     uint8_t response_byte;
//     uint8_t checksum_byte;
//     uint8_t end_byte;
// }pump_ctrl_protocol_response_buffer;

typedef struct
{
    uint8_t start_byte;
    uint8_t interface_byte;
    uint32_t response_byte;
    uint8_t checksum_byte;
    uint8_t end_byte;
}pump_ctrl_protocol_response_buffer; // 8 bytes

typedef enum
{
    PASS        = 0,
    FAILURE     = 1
}pump_protocol_flag_def;

// #define     FRAME_BUFFER_SIZE                   5
#define     FRAME_START_BYTE                    0x37
#define     FRAME_END_BYTE                      0x3F

#define 	BUFFER_SIZE							5
#define     FRAME_BUFFER_SIZE   				60
#define		PROTOCOL_FRAME_NR					12
#define     FRAME_START_BYTE    				0x37
#define     FRAME_END_BYTE      				0x3F

#define 	RESPONSE_BUFFER_SIZE				8
#define 	RESPONSE_FRAME_BUFFER_SIZE			96

typedef enum
{
    INTERFACE_EC_PUMPS      			=   0x30,
    INTERFACE_PH_PUMPS      			=   0x31,
    INTERFACE_OXY_PUMPS     			=   0x32, //
    INTERFACE_MIX_PUMP      			=   0x33, //  together will switched
    INTERFACE_M_IN_VALVE    			=   0x34,
    INTERFACE_M_OUT_VALVE   			=   0x35,
    INTERFACE_P_PUMP        			=   0x36,
    INTERFACE_GROW_LEDS     			=   0x37,
	INTERFACE_PH_SENSOR					=	0x38,
	INTERFACE_EC_SENSOR					=	0x39,
	INTERFACE_TEMPERATURE_SENSOR		=	0x3A,
	INTERFACE_DISTANCE_SENSOR			=	0x3B
}pump_protocol_interface_byteDef;

typedef enum
{
    COMMAND_ON_OR_UP        =   0x30, // motors on
    COMMAND_OFF_OR_DOWN     =   0x31,
    COMMAND_EC_UP_OFF       =   0x32,
    COMMAND_EC_DOWN_OFF     =   0x33,
    COMMAND_PH_UP_OFF       =   0x34,
    COMMAND_PH_DOWN_OFF     =   0x35    
    // COMMAND_EC_PH_OFF       =   0x32
}pump_protocol_command_byteDef;

typedef enum
{
    TRANSFER_COMP_SUCC      =   0x70,
    START_BYTE_ERROR        =   0x71,
    END_BYTE_ERROR          =   0x72,
    INTERFACE_BYTE_ERROR    =   0x73,
    COMMAND_BYTE_ERROR      =   0x74,
    CHECKSUM_BYTE_ERROR     =   0x75,
    LENGTH_ERROR            =   0x76
}pump_protocol_response_byteDef;


esp_err_t internal_uart_driver_error;
uint8_t pump_protocol_input_byte;
uint8_t pump_protocol_checksum_calc;
uint8_t pump_io_transfer_to_stm_ptr_buffer[PROTOCOL_FRAME_NR][BUFFER_SIZE];
uint8_t pump_io_transfer_to_stm_buffer[FRAME_BUFFER_SIZE];
//0 EC PUMPS 0
//1 PH PUMPS
//2 OXYGEN PUMPS
//3 MIXER MOTOR
//4 WATER IN CONTROL VALVE
//5 WATER OUT CONTROL VALVE
//6 HYDROPONIC PIPE WATERING PUMP
//7 HYDROPONIC GROWING SUNLIGHT LEDS
//8 PH SENSOR
//9 EC SENSOR
//10 TEMPERATURE SENSOR
//11 DISTANCE SENSOR
uint8_t pump_io_receive_from_stm_ptr_buffer[PROTOCOL_FRAME_NR][RESPONSE_BUFFER_SIZE];
uint8_t pump_io_receive_from_stm_buffer[RESPONSE_FRAME_BUFFER_SIZE];
pump_protocol_flag_def flag_notification;
pump_ctrl_protocol_response_buffer *PUMP_IO_COM_response;
uint8_t PUMP_IO_COM_buffer[8];
uint8_t rx_msg_placement_counter;
int PUMP_IO_COM_rx_len;

pump_protocol_flag_def diagnostics_response[PROTOCOL_FRAME_NR];

// static pump_ctrl_protocol_rcv_buffer *protocol_rcv_frame;
pump_ctrl_protocol_rcv_buffer send_cmd_data, send_cmd_data_f;

float bytesToFloat(uint8_t bytes[4]);

void IRAM_ATTR pump_io_com__uart_isr_handler(void *arg);
void pump_io_com_rx_task(void *arg);

void PUMP_IO_COM_print_pump_response(const pump_ctrl_protocol_response_buffer *msg);
void PUMP_IO_COM_print_command(const pump_ctrl_protocol_rcv_buffer *msg);

void PUMP_IO_COM_INIT_the_main_pump_ctrl(void);

pump_protocol_flag_def PUMP_IO_COM_check_len(uint8_t size_of_buffer);
pump_protocol_flag_def PUMP_IO_COM_check_start_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer);
pump_protocol_flag_def PUMP_IO_COM_check_end_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer);
pump_protocol_flag_def PUMP_IO_COM_check_interface_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer);
pump_protocol_flag_def PUMP_IO_COM_check_response_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer);
pump_protocol_flag_def PUMP_IO_COM_check_checksum_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer);

pump_protocol_flag_def PUMP_IO_COM_check_the_buffer_resp_not(const pump_ctrl_protocol_response_buffer *input_data_buffer);

void PUMP_IO_COM_send_pump_command(const pump_protocol_interface_byteDef interface, const pump_protocol_command_byteDef command);
bool PUMP_IO_COM_check_busy_flag(void);

void PUMP_IO_COM_EXE(void);


#endif //PUMP_IO_COM_H