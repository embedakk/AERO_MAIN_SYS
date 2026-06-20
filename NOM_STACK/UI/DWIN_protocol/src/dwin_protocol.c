#include "../inc/dwin_protocol.h"

//rtc
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;

static QueueHandle_t ui_protocol_uart_queue;
uart_event_t handler_event;
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
uart_event_t task_event;
uint8_t *dma_buffer;
// uint8_t *ui_protocol_input;
size_t len;

uint8_t ui_queue_data[25][21] = 
{
    {0x5a, 0xa5, 0x07, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_VIEW_PH_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_VIEW_PH_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x07, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_VIEW_EC_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_VIEW_EC_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x07, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_VIEW_W_TEMP_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_VIEW_W_TEMP_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x07, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_VIEW_DISTANCE_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_VIEW_DISTANCE_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x07, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_VIEW_TANK_CAP_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_VIEW_TANK_CAP_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x07, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_VIEW_R_TEMP_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_VIEW_R_TEMP_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x07, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_VIEW_HUMIDITY_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_VIEW_HUMIDITY_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x05, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_VIEW_FRUIT_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_VIEW_FRUIT_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x12, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_VIEW_FRUIT_TXT_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_VIEW_FRUIT_TXT_ADDRESS & 0x00FF), 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20},
    {0x5a, 0xa5, 0x06, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_VIEW_LED_STATUS_TXT_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_VIEW_LED_STATUS_TXT_ADDRESS & 0x00FF), 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x06, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_VIEW_PUMP_STATUS_TXT_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_VIEW_PUMP_STATUS_TXT_ADDRESS & 0x00FF), 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x06, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_VIEW_SYS_STATUS_TXT_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_VIEW_SYS_STATUS_TXT_ADDRESS & 0x00FF), 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x05, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_CONFIG_TANK_CAP_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_CONFIG_TANK_CAP_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x05, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_CONFIG_DOSING_INT_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_CONFIG_DOSING_INT_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x07, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_CONFIG_PH_SOLUTION_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_CONFIG_PH_SOLUTION_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x07, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_CONFIG_EC_SOLUTION_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_CONFIG_EC_SOLUTION_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x05, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_CONFIG_MIXING_TIME_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_CONFIG_MIXING_TIME_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x05, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_RTC_YEAR_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_RTC_YEAR_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x05, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_RTC_MONTH_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_RTC_MONTH_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x05, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_RTC_TIME_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_RTC_TIME_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x05, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_RTC_HOUR_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_RTC_HOUR_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x05, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_RTC_MINUTE_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_RTC_MINUTE_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x07, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_SETTING_EC_V_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_SETTING_EC_V_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x07, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_SETTING_PH_V_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_SETTING_PH_V_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x5a, 0xa5, 0x05, UI_DWIN_WRITE_ADDR, (uint8_t)(UI_DWIN_WRITE_EDIT_SYS_STAGE_ADDRESS >> 8), (uint8_t)(UI_DWIN_WRITE_EDIT_SYS_STAGE_ADDRESS & 0x00FF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

char    ui_fruit_txt[19][15] = 
{
    "BROCCOLI       ",
    "STRAWBERRY     ",
    "ROCKET         ",
    "TOMATO         ",
    "LETTUCE        ",
    "LEAK           ",
    "EGGPLANT       ",
    "PEAS           ",
    "BRUSSEL SPROUTS",
    "CABBAGE        ",
    "CHILLI         ",
    "SPINACH        ",
    "PARSLEY        ",
    "GREAN BEANS    ",
    "CUCUMBER       ",
    "KALE           ",
    "ASIAN GREANS   ",
    "UNKNOWN FRUIT  ",
    "NO SELECTION   "
};

//events
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x00, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x10, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x53, 0x20, 0x01, 0x00, 0x0B,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x10, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x30, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x00, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x00, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x10, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x10, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x20, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x30, 0x01, 0x00, 0x00,
void ui_protocol_rx_task(void *arg)
{
    while (1)
    {
 
        if (xQueueReceive(ui_protocol_uart_queue, &task_event, portMAX_DELAY)) 
        {
            if (task_event.type == UART_DATA) 
            {


                // ✅ Zero-Copy: Get DMA buffer pointer (No CPU copying!)
                uart_get_buffered_data_len(UART_NUM, &len);
                dma_buffer = (uint8_t *)malloc(len);

                if (dma_buffer) 
                {
                    uart_read_bytes(UART_NUM, dma_buffer, len, 0);
                    // ESP_LOGI(TAG, "Received (DMA Buffer):"); // %.*s", len, dma_buffer);
                    // for(uint8_t counter = 0; counter < len; counter++)
                    // {
                    //     printf("0x%02X, ", dma_buffer[counter]);
                    // }
                    // printf("\r\n");
                    //put here the touch logic
                    // ui_protocol_input = &dma_buffer;
                    parse_received_byte(len, (uint8_t *)&dma_buffer[0]);
                    free(dma_buffer);  // Free buffer after processing
                }
            }
        }
        // vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// ISR Handler (Handles UART Events)
void IRAM_ATTR ui_protocol_uart_isr_handler(void *arg) 
{


    if (xQueueReceiveFromISR(ui_protocol_uart_queue, &handler_event, &xHigherPriorityTaskWoken)) {
        if (handler_event.type == UART_DATA) {
            // ESP_LOGI(TAG, "ISR: New UART Data, Length: %d", event.size);
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Function to convert float to 4-byte IEEE 754 format
void floatToHex(float value, uint8_t *hexBytes) 
{
    union {
        float f;
        uint8_t bytes[4];
    } floatUnion;

    floatUnion.f = value;

    // DWIN expects high-byte first (Big Endian format)
    hexBytes[0] = floatUnion.bytes[3];  // MSB
    hexBytes[1] = floatUnion.bytes[2];
    hexBytes[2] = floatUnion.bytes[1];
    hexBytes[3] = floatUnion.bytes[0];  // LSB
}

void dataToDisplayBuff(uint8_t pointed_addr, float input_data, uint8_t floatBytes[4])
{
    floatToHex(input_data, floatBytes);
    ui_queue_data[pointed_addr][6] = floatBytes[0];  // 1st byte of float
    ui_queue_data[pointed_addr][7] = floatBytes[1];  // 2nd byte of float
    ui_queue_data[pointed_addr][8] = floatBytes[2];  // 3rd byte of float
    ui_queue_data[pointed_addr][9] = floatBytes[3];  // 4th byte of float

    return;
}

// Function to create a UART command for writing text
void createDwinTextCommand(char *text, uint8_t *command) 
{
    size_t len = strlen(text);

    // Copy text characters to command buffer
    for (size_t i = 0; i < len; i++) 
    {
        command[6 + i] = text[i];
    }
}

//events
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x00, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x10, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x53, 0x20, 0x01, 0x00, 0x0B,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x10, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x30, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x00, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x00, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x10, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x10, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x20, 0x01, 0x00, 0x00,
// 0x5A, 0xA5, 0x06, 0x83, 0x51, 0x30, 0x01, 0x00, 0x00,
uint8_t read_write_op_counter;
uint16_t received_data_address;
void check_address(uint8_t *received_data, uint16_t address, uint8_t format)
{
    switch (address)
    {
        case UI_DWIN_READ_SETTING_SYS_START_ADDRESS:
            {
                signal_ctrl_system_start                    =   SENSOR_IC_SIGNAL_TRUE; 
                signal_ctrl_system_stop                     =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_continue                 =   SENSOR_IC_SIGNAL_TRUE;
                signal_ctrl_system_delete                   =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_release_the_tank         =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_fill_the_tank            =   SENSOR_IC_SIGNAL_FALSE;
                signal_data_get_system_start_stop_status    =   1;
            }
            break;
        case UI_DWIN_READ_SETTING_SYS_CONTINUE_ADDRESS:
            {
                signal_ctrl_system_start                    =   SENSOR_IC_SIGNAL_FALSE; 
                signal_ctrl_system_stop                     =   SENSOR_IC_SIGNAL_TRUE;
                signal_ctrl_system_continue                 =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_delete                   =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_release_the_tank         =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_fill_the_tank            =   SENSOR_IC_SIGNAL_FALSE;
                signal_data_get_system_start_stop_status    =   2;
            }
            break;
        case UI_DWIN_READ_SETTING_SYS_DELETE_ADDRESS:
            {
                signal_ctrl_system_start                    =   SENSOR_IC_SIGNAL_FALSE; 
                signal_ctrl_system_stop                     =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_continue                 =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_delete                   =   SENSOR_IC_SIGNAL_TRUE;
                signal_ctrl_system_release_the_tank         =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_fill_the_tank            =   SENSOR_IC_SIGNAL_FALSE;
                signal_data_get_system_start_stop_status    =   3;
            }
            break;
        case UI_DWIN_READ_SETTING_SYS_STOP_ADDRESS:
            {
                signal_ctrl_system_start                    =   SENSOR_IC_SIGNAL_FALSE; 
                signal_ctrl_system_stop                     =   SENSOR_IC_SIGNAL_TRUE;
                signal_ctrl_system_continue                 =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_delete                   =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_release_the_tank         =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_fill_the_tank            =   SENSOR_IC_SIGNAL_FALSE;
                signal_data_get_system_start_stop_status    =   0;
            }
            break;
        case UI_DWIN_READ_SETTING_FILL_TANK_ADDRESS:
            {
                signal_ctrl_system_start                    =   SENSOR_IC_SIGNAL_FALSE; 
                signal_ctrl_system_stop                     =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_continue                 =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_delete                   =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_release_the_tank         =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_fill_the_tank            =   SENSOR_IC_SIGNAL_TRUE;
            }
            break;
        case UI_DWIN_READ_SETTING_RELEASE_TANK_ADDRESS:
            {
                signal_ctrl_system_start                    =   SENSOR_IC_SIGNAL_FALSE; 
                signal_ctrl_system_stop                     =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_continue                 =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_delete                   =   SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_release_the_tank         =   SENSOR_IC_SIGNAL_TRUE;
                signal_ctrl_system_fill_the_tank            =   SENSOR_IC_SIGNAL_FALSE;
            }
            break;
        case UI_DWIN_READ_CONFIG_TANK_CAP_ADDRESS:
            {
                //4 formats for 9 byte data
                //0 => 7 8
                //6 => 13 14
                //9 => 16 17
                //{5A}{A5}{06}{83}{53}{D0}{01}{00}{02}|{5A}{A5}{06}{83}{53}{D0}{01}{00}{02}
                //11 18 19
                if(format == 1)
                {
                    signal_data_get_tank_capacity = (uint16_t)((received_data[7] << 8) | received_data[8]);// 7/8  13/ 14 
                }else if(format == 2)
                {
                    signal_data_get_tank_capacity = (uint16_t)((received_data[13] << 8) | received_data[14]);// 7/8  13/ 14 
                }else if(format == 3)
                {
                    signal_data_get_tank_capacity = (uint16_t)((received_data[16] << 8) | received_data[17]);// 7/8  13/ 14 
                }else if(format == 4)
                {
                    signal_data_get_tank_capacity = (uint16_t)((received_data[18] << 8) | received_data[19]);// 7/8  13/ 14 
                }
            }
            break;
        case UI_DWIN_READ_CONFIG_DOSING_INT_ADDRESS:
            {
                if(format == 1)
                {
                    signal_data_get_dosing_intensity = (uint16_t)((received_data[7] << 8) | received_data[8]);// 7/8  13/ 14 
                }else if(format == 2)
                {
                    signal_data_get_dosing_intensity = (uint16_t)((received_data[13] << 8) | received_data[14]);// 7/8  13/ 14 
                }else if(format == 3)
                {
                    signal_data_get_dosing_intensity = (uint16_t)((received_data[16] << 8) | received_data[17]);// 7/8  13/ 14 
                }else if(format == 4)
                {
                    signal_data_get_dosing_intensity = (uint16_t)((received_data[18] << 8) | received_data[19]);// 7/8  13/ 14 
                }
            }
            break;
        case UI_DWIN_READ_CONFIG_MIXING_TIME_ADDRESS:
            {
                if(format == 1)
                {
                    signal_data_get_mixing_time = (uint16_t)((received_data[7] << 8) | received_data[8]);// 7/8  13/ 14 
                }else if(format == 2)
                {
                    signal_data_get_mixing_time = (uint16_t)((received_data[13] << 8) | received_data[14]);// 7/8  13/ 14 
                }else if(format == 3)
                {
                    signal_data_get_mixing_time = (uint16_t)((received_data[16] << 8) | received_data[17]);// 7/8  13/ 14 
                }else if(format == 4)
                {
                    signal_data_get_mixing_time = (uint16_t)((received_data[18] << 8) | received_data[19]);// 7/8  13/ 14 
                }
            }
            break;
        case UI_DWIN_READ_RTC_YEAR_ADDRESS:
            {
                // uint16_t year_data;
                if(format == 1)
                {
                    // year_data = (uint16_t)((received_data[7] << 8) | received_data[8]);//
                    signal_data_set_rtc_data->year = (uint16_t)((received_data[7] << 8) | received_data[8]);// 7/8  13/ 14 
                }else if(format == 2)
                {
                    // year_data = (uint16_t)((received_data[13] << 8) | received_data[14]);//
                    signal_data_set_rtc_data->year = (uint16_t)((received_data[13] << 8) | received_data[14]);// 7/8  13/ 14 
                }else if(format == 3)
                {
                    signal_data_set_rtc_data->year = (uint16_t)((received_data[16] << 8) | received_data[17]);// 7/8  13/ 14 
                }else if(format == 4)
                {
                    signal_data_set_rtc_data->year = (uint16_t)((received_data[18] << 8) | received_data[19]);// 7/8  13/ 14 
                }
                signal_ctrl_rtc_activate_set_rtc            =   SENSOR_IC_SIGNAL_TRUE;
            }
            break;
        case UI_DWIN_READ_RTC_MONTH_ADDRESS:
            {
                if(format == 1)
                {
                    signal_data_set_rtc_data->month = (uint8_t)((received_data[7] << 8) | received_data[8]);// 7/8  13/ 14 
                }else if(format == 2)
                {
                    signal_data_set_rtc_data->month = (uint8_t)((received_data[13] << 8) | received_data[14]);// 7/8  13/ 14 
                }else if(format == 3)
                {
                    signal_data_set_rtc_data->month = (uint8_t)((received_data[16] << 8) | received_data[17]);// 7/8  13/ 14 
                }else if(format == 4)
                {
                    signal_data_set_rtc_data->month = (uint8_t)((received_data[18] << 8) | received_data[19]);// 7/8  13/ 14 
                }
                signal_ctrl_rtc_activate_set_rtc            =   SENSOR_IC_SIGNAL_TRUE;
            }
            break;
        case UI_DWIN_READ_RTC_TIME_ADDRESS:
            {
                if(format == 1)
                {
                    signal_data_set_rtc_data->dayMonth = (uint8_t)((received_data[7] << 8) | received_data[8]);// 7/8  13/ 14 
                }else if(format == 2)
                {
                    signal_data_set_rtc_data->dayMonth = (uint8_t)((received_data[13] << 8) | received_data[14]);// 7/8  13/ 14 
                }else if(format == 3)
                {
                    signal_data_set_rtc_data->dayMonth = (uint8_t)((received_data[16] << 8) | received_data[17]);// 7/8  13/ 14 
                }else if(format == 4)
                {
                    signal_data_set_rtc_data->dayMonth = (uint8_t)((received_data[18] << 8) | received_data[19]);// 7/8  13/ 14 
                }
                signal_ctrl_rtc_activate_set_rtc            =   SENSOR_IC_SIGNAL_TRUE;
            }
            break;
        case UI_DWIN_READ_RTC_HOUR_ADDRESS:
            {
                if(format == 1)
                {
                    signal_data_set_rtc_data->hour = (uint8_t)((received_data[7] << 8) | received_data[8]);// 7/8  13/ 14 
                }else if(format == 2)
                {
                    signal_data_set_rtc_data->hour = (uint8_t)((received_data[13] << 8) | received_data[14]);// 7/8  13/ 14 
                }else if(format == 3)
                {
                    signal_data_set_rtc_data->hour = (uint8_t)((received_data[16] << 8) | received_data[17]);// 7/8  13/ 14 
                }else if(format == 4)
                {
                    signal_data_set_rtc_data->hour = (uint8_t)((received_data[18] << 8) | received_data[19]);// 7/8  13/ 14 
                } 
                signal_ctrl_rtc_activate_set_rtc            =   SENSOR_IC_SIGNAL_TRUE;
            }
            break;
        case UI_DWIN_READ_RTC_MINUTE_ADDRESS:
            {
                if(format == 1)
                {
                    signal_data_set_rtc_data->minute = (uint8_t)((received_data[7] << 8) | received_data[8]);// 7/8  13/ 14 
                }else if(format == 2)
                {
                    signal_data_set_rtc_data->minute = (uint8_t)((received_data[13] << 8) | received_data[14]);// 7/8  13/ 14 
                }else if(format == 3)
                {
                    signal_data_set_rtc_data->minute = (uint8_t)((received_data[16] << 8) | received_data[17]);// 7/8  13/ 14 
                }else if(format == 4)
                {
                    signal_data_set_rtc_data->minute = (uint8_t)((received_data[18] << 8) | received_data[19]);// 7/8  13/ 14 
                }
                signal_ctrl_rtc_activate_set_rtc            =   SENSOR_IC_SIGNAL_TRUE;
            }
            break;
        case UI_DWIN_READ_SETTING_EC_CAL_L_ADDRESS:
            {
                signal_ctrl_system_cal_ec_low               =   SENSOR_IC_SIGNAL_TRUE;
            }
            break;
        case UI_DWIN_READ_SETTING_EC_CAL_M_ADDRESS:
            {
                signal_ctrl_system_cal_ec_mid               =   SENSOR_IC_SIGNAL_TRUE; 
                //assign mid calibrations voltage value  
                signal_data_set_ec_cal_mid_V                =   signal_data_set_ec_voltage;           
            }
            break;
        case UI_DWIN_READ_SETTING_EC_CAL_H_ADDRESS:
            {
                signal_ctrl_system_cal_ec_high              =   SENSOR_IC_SIGNAL_TRUE;
            }
            break;
        case UI_DWIN_READ_SETTING_PH_CAL_L_ADDRESS:
            {
                signal_ctrl_system_cal_ph_low               =   SENSOR_IC_SIGNAL_TRUE;
                signal_data_set_ph_cal_low_V                =   signal_data_set_ph_voltage;
            }
            break;
        case UI_DWIN_READ_SETTING_PH_CAL_M_ADDRESS:
            {
                signal_ctrl_system_cal_ph_mid               =   SENSOR_IC_SIGNAL_TRUE;                
                signal_data_set_ph_cal_mid_V                =   signal_data_set_ph_voltage;
            }
            break;
        case UI_DWIN_READ_SETTING_PH_CAL_H_ADDRESS:
            {
                signal_ctrl_system_cal_ph_high              =   SENSOR_IC_SIGNAL_TRUE;
                signal_data_set_ph_cal_high_V               =   signal_data_set_ph_voltage;
            }
            break;
        case UI_DWIN_READ_EDIT_SYS_STAGE_ADDRESS:
            {
                if(format == 1)
                {
                    signal_data_set_system_stage_data = (uint8_t)(received_data[8]);// 7/8  13/ 14 
                }else if(format == 2)
                {
                    signal_data_set_system_stage_data = (uint8_t)(received_data[14]);// 7/8  13/ 14 
                }else if(format == 3)
                {
                    signal_data_set_system_stage_data = (uint8_t)(received_data[17]);// 7/8  13/ 14 
                }else if(format == 4)
                {
                    signal_data_set_system_stage_data = (uint8_t)(received_data[19]);// 7/8  13/ 14 
                }
            }
            break;
        case UI_DWIN_READ_EDIT_BROCCOLI_ADDRESS:
            {
                signal_data_set_selected_fruit = 0;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_STRAWBERRY_ADDRESS:
            {
                signal_data_set_selected_fruit = 1;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_ROCKET_ADDRESS:
            {
                signal_data_set_selected_fruit = 2;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_TOMATO_ADDRESS:
            {
                signal_data_set_selected_fruit = 3;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_LETTUCE_ADDRESS:
            {
                signal_data_set_selected_fruit = 4;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_LEAK_ADDRESS:
            {
                signal_data_set_selected_fruit = 5;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_EGGPLANT_ADDRESS:
            {
                signal_data_set_selected_fruit = 6;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_PEAS_ADDRESS:
            {
                signal_data_set_selected_fruit = 7;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_BR_SPROUTS_ADDRESS:
            {
                signal_data_set_selected_fruit = 8;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_CABBAGE_ADDRESS:
            {
                signal_data_set_selected_fruit = 9;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_CHILLI_ADDRESS:
            {
                signal_data_set_selected_fruit = 10;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_SPINACH_ADDRESS:
            {
                signal_data_set_selected_fruit = 11;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_PARSLEY_ADDRESS:
            {
                signal_data_set_selected_fruit = 12;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_GREEN_BEANS_ADDRESS:
            {
                signal_data_set_selected_fruit = 13;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_CUCUMBER_ADDRESS:
            {
                signal_data_set_selected_fruit = 14;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_KALE_ADDRESS:
            {
                signal_data_set_selected_fruit = 15;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        case UI_DWIN_READ_EDIT_ASIAN_GREANS_ADDRESS:
            {
                signal_data_set_selected_fruit = 16;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
            }
            break;
        
        default:
            {
                signal_data_set_selected_fruit = 0x17;
                signal_data_set_text_selected_fruit[0]  = ui_fruit_txt[signal_data_set_selected_fruit][0];
                signal_data_set_text_selected_fruit[1]  = ui_fruit_txt[signal_data_set_selected_fruit][1];
                signal_data_set_text_selected_fruit[2]  = ui_fruit_txt[signal_data_set_selected_fruit][2];
                signal_data_set_text_selected_fruit[3]  = ui_fruit_txt[signal_data_set_selected_fruit][3];
                signal_data_set_text_selected_fruit[4]  = ui_fruit_txt[signal_data_set_selected_fruit][4];
                signal_data_set_text_selected_fruit[5]  = ui_fruit_txt[signal_data_set_selected_fruit][5];
                signal_data_set_text_selected_fruit[6]  = ui_fruit_txt[signal_data_set_selected_fruit][6];
                signal_data_set_text_selected_fruit[7]  = ui_fruit_txt[signal_data_set_selected_fruit][7];
                signal_data_set_text_selected_fruit[8]  = ui_fruit_txt[signal_data_set_selected_fruit][8];
                signal_data_set_text_selected_fruit[9]  = ui_fruit_txt[signal_data_set_selected_fruit][9];
                signal_data_set_text_selected_fruit[10] = ui_fruit_txt[signal_data_set_selected_fruit][10];
                signal_data_set_text_selected_fruit[11] = ui_fruit_txt[signal_data_set_selected_fruit][11];
                signal_data_set_text_selected_fruit[12] = ui_fruit_txt[signal_data_set_selected_fruit][12];
                signal_data_set_text_selected_fruit[13] = ui_fruit_txt[signal_data_set_selected_fruit][13];
                signal_data_set_text_selected_fruit[14] = ui_fruit_txt[signal_data_set_selected_fruit][14];
                //error
            }
            break;
    }
}

void parse_received_byte(size_t length_of_data, uint8_t *received_data)
{
    if((length_of_data - 3) == received_data[2])
    {
        if(received_data[2] == 0x03)
        {
            //response to write operation
            read_write_op_counter--;
        }else if(received_data[2] == 0x06)
        {
            
            received_data_address = (uint16_t)((received_data[4] << 8) | received_data[5]);
            check_address(received_data, received_data_address, 1);
        }else if(received_data[2] == 0x08)
        {
            received_data_address = (uint16_t)((received_data[4] << 8) | received_data[5]);
            if(received_data_address == UI_DWIN_READ_CONFIG_PH_SOLUTION_ADDRESS)
            {
                signal_data_get_ph_concentration_data = (uint32_t)((received_data[7] << 24) | (received_data[8] << 16) 
                                                                 | (received_data[9] << 8)  | (received_data[10]));
            }else if (received_data_address == UI_DWIN_READ_CONFIG_EC_SOLUTION_ADDRESS)
            {
                signal_data_get_ec_concentration_data = (uint32_t)((received_data[7] << 24) | (received_data[8] << 16) 
                                                                 | (received_data[9] << 8)  | (received_data[10]));
            }else
            {
                //error
                return;
            }          
        }else
        {
            //len error
            return;
        }
    }else
    {
        if(received_data[2] == 0x03)
        {
            //response to write operation
            read_write_op_counter--;
            if(received_data[8] == 0x03)
            {
                //response to write operation
                read_write_op_counter--;       
            }else if(received_data[8] == 0x06)
            {
                received_data_address = (uint16_t)((received_data[10] << 8) | received_data[11]);
                check_address(received_data, received_data_address, 2);
            }else if(received_data[8] == 0x08)
            {
                received_data_address = (uint16_t)((received_data[10] << 8) | received_data[11]);
                if(received_data_address == UI_DWIN_READ_CONFIG_PH_SOLUTION_ADDRESS)
                {
                    signal_data_get_ph_concentration_data = (uint32_t)((received_data[13] << 24) | (received_data[14] << 16) 
                                                                     | (received_data[15] << 8)  | (received_data[16]));
                }else if (received_data_address == UI_DWIN_READ_CONFIG_EC_SOLUTION_ADDRESS)
                {
                    signal_data_get_ec_concentration_data = (uint32_t)((received_data[13] << 24) | (received_data[14] << 16) 
                                                                     | (received_data[15] << 8)  | (received_data[16]));
                }else
                {
                    //error
                    return;
                } 
            }
        }else if(received_data[2] == 0x06)
        {
            received_data_address = (uint16_t)((received_data[4] << 8) | received_data[5]);
            check_address(received_data, received_data_address, 2);
            if(received_data[11] == 0x03)
            {
                //response to write operation
                read_write_op_counter--;       
            }else if(received_data[11] == 0x06)
            {
                received_data_address = (uint16_t)((received_data[13] << 8) | received_data[14]);
                check_address(received_data, received_data_address, 3);
            }else if(received_data[11] == 0x08)
            {
                received_data_address = (uint16_t)((received_data[13] << 8) | received_data[14]);
                if(received_data_address == UI_DWIN_READ_CONFIG_PH_SOLUTION_ADDRESS)
                {
                    signal_data_get_ph_concentration_data = (uint32_t)((received_data[16] << 24) | (received_data[17] << 16) 
                                                                     | (received_data[18] << 8)  | (received_data[19]));
                }else if (received_data_address == UI_DWIN_READ_CONFIG_EC_SOLUTION_ADDRESS)
                {
                    signal_data_get_ec_concentration_data = (uint32_t)((received_data[16] << 24) | (received_data[17] << 16) 
                                                                     | (received_data[18] << 8)  | (received_data[19]));
                }else
                {
                    //error
                    return;
                } 
            }
        }else if(received_data[2] == 0x08)
        {
            received_data_address = (uint16_t)((received_data[4] << 8) | received_data[5]);
            if(received_data_address == UI_DWIN_READ_CONFIG_PH_SOLUTION_ADDRESS)
            {
                signal_data_get_ph_concentration_data = (uint32_t)((received_data[7] << 24) | (received_data[8] << 16) 
                                                                 | (received_data[9] << 8)  | (received_data[10]));
            }else if (received_data_address == UI_DWIN_READ_CONFIG_EC_SOLUTION_ADDRESS)
            {
                signal_data_get_ec_concentration_data = (uint32_t)((received_data[7] << 24) | (received_data[8] << 16) 
                                                                 | (received_data[9] << 8)  | (received_data[10]));          
            }else
            {
                //error
                return;
            }  
            if(received_data[13] == 0x03)
            {
                //response to write operation
                read_write_op_counter--;       
            }else if(received_data[13] == 0x06)
            {
                received_data_address = (uint16_t)((received_data[15] << 8) | received_data[16]);
                check_address(received_data, received_data_address, 4);
            }else if(received_data[13] == 0x08)
            {
                received_data_address = (uint16_t)((received_data[15] << 8) | received_data[16]);
                if(received_data_address == UI_DWIN_READ_CONFIG_PH_SOLUTION_ADDRESS)
                {
                    signal_data_get_ph_concentration_data = (uint32_t)((received_data[18] << 24) | (received_data[19] << 16) 
                                                                     | (received_data[20] << 8)  | (received_data[21]));
                }else if (received_data_address == UI_DWIN_READ_CONFIG_EC_SOLUTION_ADDRESS)
                {
                    signal_data_get_ec_concentration_data = (uint32_t)((received_data[18] << 24) | (received_data[19] << 16) 
                                                                     | (received_data[20] << 8)  | (received_data[21])); 
                }else
                {
                    //error
                    return;
                } 
            }
        }else
        {
            //len error
            return;
        }
    }
}

void UI_INIT(void)
{
        // UART Configuration
        uart_config_t uart_config = 
        {
            .baud_rate = 115200, //115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
        };
    
        uart_param_config(UART_NUM, &uart_config);
        uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
        // Enable UART Driver with DMA (Double Buffer)
        uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &ui_protocol_uart_queue, 0);
        uart_enable_rx_intr(UART_NUM);
    
        // ESP_LOGI(TAG, "UART DMA RX Initialized!");
        xTaskCreate(ui_protocol_rx_task, "ui_protocol_rx_task", 1024*2, NULL, 23, NULL);
        return;
}

uint8_t ui_protocol_counter = 0;
void UI_DATA_OUT(void)
{
    //UART out buffer values to display cyclicly
    uart_write_bytes(UART_NUM, &ui_queue_data[ui_protocol_counter][0], (ui_queue_data[ui_protocol_counter][2] + 3));
    read_write_op_counter++;//use that for response checking logic
    if (ui_protocol_counter < 24)
    {
        ui_protocol_counter++;
    }else
    {
        ui_protocol_counter = 0;
    }
    return;
}

void UI_EXE(void)
{
    //put the signal data's to buffer
    // floatToHex(signal_data_get_ph_data, ph_conversion_bytes);
    // ui_queue_data[0][6] = ph_conversion_bytes[0];  // 1st byte of float
    // ui_queue_data[0][7] = ph_conversion_bytes[1];  // 2nd byte of float
    // ui_queue_data[0][8] = ph_conversion_bytes[2];  // 3rd byte of float
    // ui_queue_data[0][9] = ph_conversion_bytes[3];  // 4th byte of float
    dataToDisplayBuff(0, signal_data_get_ph_data, ph_conversion_bytes);
    dataToDisplayBuff(1, signal_data_get_ec_data, ec_conversion_bytes);
    dataToDisplayBuff(2, signal_data_get_water_temperature_data, water_temp_conversion_bytes);
    dataToDisplayBuff(3, signal_data_get_water_level_data, distance_conversion_bytes);
    dataToDisplayBuff(4, signal_data_get_water_cap_percentage, tank_cap_percentage_conversion_bytes);
    dataToDisplayBuff(5, signal_data_get_room_temperature_data, room_temp_conversion_bytes);
    dataToDisplayBuff(6, signal_data_get_room_humidity_data, humidity_conversion_bytes);

    ui_queue_data[7][6] = (uint8_t)(signal_data_set_selected_fruit >> 8);  // 1st byte 
    ui_queue_data[7][7] = (uint8_t)(signal_data_set_selected_fruit & 0x00FF);// 2nd byte 

    createDwinTextCommand(&signal_data_set_text_selected_fruit[0], &ui_queue_data[8][0]);
    createDwinTextCommand(&signal_data_set_text_led_status[0], &ui_queue_data[9][0]); 
    createDwinTextCommand(&signal_data_set_text_pump_status[0], &ui_queue_data[10][0]); 
    createDwinTextCommand(&signal_data_set_text_sys_status[0], &ui_queue_data[11][0]); 

    ui_queue_data[12][6] = (uint8_t)(signal_data_get_tank_capacity >> 8);  // 1st byte 
    ui_queue_data[12][7] = (uint8_t)(signal_data_get_tank_capacity & 0x00FF);// 2nd byte
  
    ui_queue_data[13][6] = (uint8_t)(signal_data_get_dosing_intensity >> 8);  // 1st byte
    ui_queue_data[13][7] = (uint8_t)(signal_data_get_dosing_intensity & 0x00FF);// 2nd byte
  
    ui_queue_data[14][6] = (uint8_t)(signal_data_get_ph_concentration_data >> 24);  // 1st byte
    ui_queue_data[14][7] = (uint8_t)(signal_data_get_ph_concentration_data >> 16);// 2nd byte 
    ui_queue_data[14][8] = (uint8_t)(signal_data_get_ph_concentration_data >> 8);  // 3rd byte 
    ui_queue_data[14][9] = (uint8_t)(signal_data_get_ph_concentration_data & 0x000000FF);// 4th byte 

    ui_queue_data[15][6] = (uint8_t)(signal_data_get_ec_concentration_data >> 24);  // 1st byte 
    ui_queue_data[15][7] = (uint8_t)(signal_data_get_ec_concentration_data >> 16);// 2nd byte 
    ui_queue_data[15][8] = (uint8_t)(signal_data_get_ec_concentration_data >> 8);  // 3rd byte 
    ui_queue_data[15][9] = (uint8_t)(signal_data_get_ec_concentration_data & 0x000000FF);// 4th byte 

    ui_queue_data[16][6] = (uint8_t)(signal_data_get_mixing_time >> 8);  
    ui_queue_data[16][7] = (uint8_t)(signal_data_get_mixing_time & 0x00FF);

    ui_queue_data[17][6] = (uint8_t)(signal_data_get_rtc_data->year >> 8);  
    ui_queue_data[17][7] = (uint8_t)(signal_data_get_rtc_data->year & 0x00FF);

    ui_queue_data[18][7] = signal_data_get_rtc_data->month;  
    ui_queue_data[19][7] = signal_data_get_rtc_data->dayMonth;  
    ui_queue_data[20][7] = signal_data_get_rtc_data->hour;  
    ui_queue_data[21][7] = signal_data_get_rtc_data->minute;  

    dataToDisplayBuff(22, signal_data_set_ec_voltage, ec_voltage_conversion_bytes);
    dataToDisplayBuff(23, signal_data_set_ph_voltage, ph_voltage_conversion_bytes);

    ui_queue_data[24][7] = signal_data_set_system_stage_data;

    // printf("get_year%d get month %d\r\n", signal_data_get_rtc_data->year, signal_data_get_rtc_data->month);
    // signal_ctrl_rtc_activate_set_rtc = SENSOR_IC_SIGNAL_TRUE;
    // printf("Error Counter %d\r\n", read_write_op_counter);
}
