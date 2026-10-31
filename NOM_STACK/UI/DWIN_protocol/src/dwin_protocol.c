#include "../inc/dwin_protocol.h"

uint8_t ph_conversion_bytes[4] = {0};
uint8_t ec_conversion_bytes[4] = {0};
uint8_t water_temp_conversion_bytes[4] = {0};
uint8_t distance_conversion_bytes[4] = {0};
uint8_t tank_cap_percentage_conversion_bytes[4] = {0};
uint8_t room_temp_conversion_bytes[4] = {0};
uint8_t humidity_conversion_bytes[4] = {0};
uint8_t ph_voltage_conversion_bytes[4] = {0};
uint8_t ec_voltage_conversion_bytes[4] = {0};

static QueueHandle_t ui_protocol_uart_queue = NULL;
static uart_event_t task_event;

uint8_t read_write_op_counter = 0;
uint16_t received_data_address = 0;
uint8_t ui_protocol_counter = 0;

#define DWIN_RX_STREAM_CAP 256U
#define DWIN_HEADER_BYTE_0 0x5AU
#define DWIN_HEADER_BYTE_1 0xA5U

static uint8_t s_dwin_rx_buf[DWIN_RX_STREAM_CAP];
static size_t s_dwin_rx_len = 0;

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

void UI_reset_parser(void)
{
    s_dwin_rx_len = 0;
    memset(s_dwin_rx_buf, 0, sizeof(s_dwin_rx_buf));
}

void ui_protocol_rx_task(void *arg)
{
    (void)arg;
    uint8_t rx_chunk[128];

    while (1)
    {
        if (xQueueReceive(ui_protocol_uart_queue, &task_event, portMAX_DELAY)) 
        {
            if (task_event.type == UART_FIFO_OVF ||
                task_event.type == UART_BUFFER_FULL)
            {
                uart_flush_input(UART_NUM);
                xQueueReset(ui_protocol_uart_queue);
                SIGNALS_lock();
                UI_reset_parser();
                SIGNALS_unlock();
            }
            else if (task_event.type == UART_DATA) 
            {
                size_t buffered_len = 0;
                uart_get_buffered_data_len(UART_NUM, &buffered_len);
                while (buffered_len > 0U) {
                    size_t to_read = (buffered_len > sizeof(rx_chunk)) ? sizeof(rx_chunk) : buffered_len;
                    int read_bytes = uart_read_bytes(UART_NUM, rx_chunk, (uint32_t)to_read, 0);
                    if (read_bytes <= 0) {
                        break;
                    }
                    SIGNALS_lock();
                    parse_received_byte((size_t)read_bytes, rx_chunk);
                    SIGNALS_unlock();
                    if ((size_t)read_bytes >= buffered_len) {
                        buffered_len = 0U;
                    } else {
                        buffered_len -= (size_t)read_bytes;
                    }
                }
            }
        }
    }
}

// Function to convert float to 4-byte IEEE 754 format (Big Endian)
void floatToHex(float value, uint8_t *hexBytes) 
{
    if (hexBytes == NULL) {
        return;
    }

    union {
        float f;
        uint8_t bytes[4];
    } floatUnion;

    floatUnion.f = isfinite(value) ? value : 0.0f;

    hexBytes[0] = floatUnion.bytes[3];  // MSB
    hexBytes[1] = floatUnion.bytes[2];
    hexBytes[2] = floatUnion.bytes[1];
    hexBytes[3] = floatUnion.bytes[0];  // LSB
}

void dataToDisplayBuff(uint8_t pointed_addr, float input_data, uint8_t floatBytes[4])
{
    if (pointed_addr >= 25U || floatBytes == NULL) {
        return;
    }
    floatToHex(input_data, floatBytes);
    ui_queue_data[pointed_addr][6] = floatBytes[0];
    ui_queue_data[pointed_addr][7] = floatBytes[1];
    ui_queue_data[pointed_addr][8] = floatBytes[2];
    ui_queue_data[pointed_addr][9] = floatBytes[3];
}

// Bounded, space-padded text formatter for DWIN VP text commands
void createDwinTextCommand(char *text, uint8_t *command) 
{
    if (text == NULL || command == NULL) {
        return;
    }

    /* Payload length is command[2] = 3 (cmd + addr_hi + addr_lo) + text_capacity */
    size_t max_chars = 0U;
    if (command[2] > 3U) {
        max_chars = (size_t)(command[2] - 3U);
        if (max_chars > 15U) {
            max_chars = 15U;
        }
    }

    bool reached_end = false;
    for (size_t i = 0U; i < max_chars; i++) 
    {
        if (!reached_end && text[i] != '\0') {
            command[6U + i] = (uint8_t)text[i];
        } else {
            reached_end = true;
            command[6U + i] = 0x20U; /* Pad with ASCII space */
        }
    }
}

static void ensure_rtc_set_seeded(void)
{
    if (signal_data_set_rtc_data == NULL) {
        signal_data_set_rtc_data = &ref_set_rtc_data;
    }
    if (signal_data_get_rtc_data == NULL) {
        signal_data_get_rtc_data = &ref_get_rtc_data;
    }
    if (signal_ctrl_rtc_activate_set_rtc != SENSOR_IC_SIGNAL_TRUE) {
        *signal_data_set_rtc_data = *signal_data_get_rtc_data;
    }
}

static void dispatch_dwin_vp_value(uint16_t address, const uint8_t *val_bytes, size_t val_len)
{
    if (val_bytes == NULL || val_len < 2U) {
        return;
    }

    received_data_address = address;
    uint16_t u16_val = (uint16_t)(((uint16_t)val_bytes[0] << 8) | val_bytes[1]);

    if (address == UI_DWIN_READ_CONFIG_PH_SOLUTION_ADDRESS) {
        if (val_len >= 4U) {
            uint32_t u32_val = ((uint32_t)val_bytes[0] << 24) |
                               ((uint32_t)val_bytes[1] << 16) |
                               ((uint32_t)val_bytes[2] << 8)  |
                               ((uint32_t)val_bytes[3]);
            if (u32_val < 1UL) u32_val = 1UL;
            if (u32_val > 1000000UL) u32_val = 1000000UL;
            signal_data_get_ph_concentration_data = u32_val;
        }
        return;
    }

    if (address == UI_DWIN_READ_CONFIG_EC_SOLUTION_ADDRESS) {
        if (val_len >= 4U) {
            uint32_t u32_val = ((uint32_t)val_bytes[0] << 24) |
                               ((uint32_t)val_bytes[1] << 16) |
                               ((uint32_t)val_bytes[2] << 8)  |
                               ((uint32_t)val_bytes[3]);
            if (u32_val < 1UL) u32_val = 1UL;
            if (u32_val > 10000000UL) u32_val = 10000000UL;
            signal_data_get_ec_concentration_data = u32_val;
        }
        return;
    }

    switch (address)
    {
        case UI_DWIN_READ_SETTING_SYS_START_ADDRESS:
            if (signal_data_set_selected_fruit < 17U) {
                signal_ctrl_system_start                    = SENSOR_IC_SIGNAL_TRUE; 
                signal_ctrl_system_stop                     = SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_continue                 = SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_delete                   = SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_release_the_tank         = SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_fill_the_tank            = SENSOR_IC_SIGNAL_FALSE;
                signal_data_get_system_start_stop_status    = 1U;
            } else {
                /* Reject Start when no valid crop is selected */
                signal_ctrl_system_start                    = SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_continue                 = SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_stop                     = SENSOR_IC_SIGNAL_TRUE;
                signal_data_get_system_start_stop_status    = 0U;
            }
            break;

        case UI_DWIN_READ_SETTING_SYS_CONTINUE_ADDRESS:
            if (signal_data_set_selected_fruit < 17U) {
                signal_ctrl_system_start                    = SENSOR_IC_SIGNAL_TRUE; 
                signal_ctrl_system_stop                     = SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_continue                 = SENSOR_IC_SIGNAL_TRUE;
                signal_ctrl_system_delete                   = SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_release_the_tank         = SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_fill_the_tank            = SENSOR_IC_SIGNAL_FALSE;
                signal_data_get_system_start_stop_status    = 2U;
            } else {
                signal_ctrl_system_start                    = SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_continue                 = SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_system_stop                     = SENSOR_IC_SIGNAL_TRUE;
                signal_data_get_system_start_stop_status    = 0U;
            }
            break;

        case UI_DWIN_READ_SETTING_SYS_DELETE_ADDRESS:
            signal_ctrl_system_start                    = SENSOR_IC_SIGNAL_FALSE; 
            signal_ctrl_system_stop                     = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_continue                 = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_delete                   = SENSOR_IC_SIGNAL_TRUE;
            signal_ctrl_system_release_the_tank         = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_fill_the_tank            = SENSOR_IC_SIGNAL_FALSE;
            signal_data_get_system_start_stop_status    = 3U;
            break;

        case UI_DWIN_READ_SETTING_SYS_STOP_ADDRESS:
            signal_ctrl_system_start                    = SENSOR_IC_SIGNAL_FALSE; 
            signal_ctrl_system_stop                     = SENSOR_IC_SIGNAL_TRUE;
            signal_ctrl_system_continue                 = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_delete                   = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_release_the_tank         = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_fill_the_tank            = SENSOR_IC_SIGNAL_FALSE;
            signal_data_get_system_start_stop_status    = 0U;
            break;

        case UI_DWIN_READ_SETTING_FILL_TANK_ADDRESS:
            signal_ctrl_system_start                    = SENSOR_IC_SIGNAL_FALSE; 
            signal_ctrl_system_stop                     = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_continue                 = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_delete                   = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_release_the_tank         = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_fill_the_tank            = SENSOR_IC_SIGNAL_TRUE;
            signal_data_get_system_start_stop_status    = 0U;
            break;

        case UI_DWIN_READ_SETTING_RELEASE_TANK_ADDRESS:
            signal_ctrl_system_start                    = SENSOR_IC_SIGNAL_FALSE; 
            signal_ctrl_system_stop                     = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_continue                 = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_delete                   = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_release_the_tank         = SENSOR_IC_SIGNAL_TRUE;
            signal_ctrl_system_fill_the_tank            = SENSOR_IC_SIGNAL_FALSE;
            signal_data_get_system_start_stop_status    = 0U;
            break;

        case UI_DWIN_READ_CONFIG_TANK_CAP_ADDRESS:
            if (u16_val < 1U) u16_val = 1U;
            if (u16_val > 5000U) u16_val = 5000U;
            signal_data_get_tank_capacity = u16_val;
            break;

        case UI_DWIN_READ_CONFIG_DOSING_INT_ADDRESS:
            if (u16_val < 1U) u16_val = 1U;
            if (u16_val > 1000U) u16_val = 1000U;
            signal_data_get_dosing_intensity = u16_val;
            break;

        case UI_DWIN_READ_CONFIG_MIXING_TIME_ADDRESS:
            if (u16_val < 10U) u16_val = 10U;
            if (u16_val > 3600U) u16_val = 3600U;
            signal_data_get_mixing_time = u16_val;
            break;

        case UI_DWIN_READ_RTC_YEAR_ADDRESS:
            if (u16_val >= 2000U && u16_val <= 2099U) {
                ensure_rtc_set_seeded();
                signal_data_set_rtc_data->year = u16_val;
                signal_ctrl_rtc_activate_set_rtc = SENSOR_IC_SIGNAL_TRUE;
            }
            break;

        case UI_DWIN_READ_RTC_MONTH_ADDRESS:
            if (u16_val >= 1U && u16_val <= 12U) {
                ensure_rtc_set_seeded();
                signal_data_set_rtc_data->month = (uint8_t)u16_val;
                signal_ctrl_rtc_activate_set_rtc = SENSOR_IC_SIGNAL_TRUE;
            }
            break;

        case UI_DWIN_READ_RTC_TIME_ADDRESS:
            if (u16_val >= 1U && u16_val <= 31U) {
                ensure_rtc_set_seeded();
                signal_data_set_rtc_data->dayMonth = (uint8_t)u16_val;
                signal_ctrl_rtc_activate_set_rtc = SENSOR_IC_SIGNAL_TRUE;
            }
            break;

        case UI_DWIN_READ_RTC_HOUR_ADDRESS:
            if (u16_val <= 23U) {
                ensure_rtc_set_seeded();
                signal_data_set_rtc_data->hour = (uint8_t)u16_val;
                signal_ctrl_rtc_activate_set_rtc = SENSOR_IC_SIGNAL_TRUE;
            }
            break;

        case UI_DWIN_READ_RTC_MINUTE_ADDRESS:
            if (u16_val <= 59U) {
                ensure_rtc_set_seeded();
                signal_data_set_rtc_data->minute = (uint8_t)u16_val;
                signal_ctrl_rtc_activate_set_rtc = SENSOR_IC_SIGNAL_TRUE;
            }
            break;

        case UI_DWIN_READ_SETTING_EC_CAL_L_ADDRESS:
            signal_ctrl_system_cal_ec_low = SENSOR_IC_SIGNAL_TRUE;
            if (isfinite(signal_data_set_ec_voltage)) {
                signal_data_set_ec_cal_low_V = signal_data_set_ec_voltage;
            }
            break;

        case UI_DWIN_READ_SETTING_EC_CAL_M_ADDRESS:
            signal_ctrl_system_cal_ec_mid = SENSOR_IC_SIGNAL_TRUE; 
            if (isfinite(signal_data_set_ec_voltage)) {
                signal_data_set_ec_cal_mid_V = signal_data_set_ec_voltage;
            }
            break;

        case UI_DWIN_READ_SETTING_EC_CAL_H_ADDRESS:
            signal_ctrl_system_cal_ec_high = SENSOR_IC_SIGNAL_TRUE;
            if (isfinite(signal_data_set_ec_voltage)) {
                signal_data_set_ec_cal_high_V = signal_data_set_ec_voltage;
            }
            break;

        case UI_DWIN_READ_SETTING_PH_CAL_L_ADDRESS:
            signal_ctrl_system_cal_ph_low = SENSOR_IC_SIGNAL_TRUE;
            if (isfinite(signal_data_set_ph_voltage)) {
                signal_data_set_ph_cal_low_V = signal_data_set_ph_voltage;
            }
            break;

        case UI_DWIN_READ_SETTING_PH_CAL_M_ADDRESS:
            signal_ctrl_system_cal_ph_mid = SENSOR_IC_SIGNAL_TRUE;                
            if (isfinite(signal_data_set_ph_voltage)) {
                signal_data_set_ph_cal_mid_V = signal_data_set_ph_voltage;
            }
            break;

        case UI_DWIN_READ_SETTING_PH_CAL_H_ADDRESS:
            signal_ctrl_system_cal_ph_high = SENSOR_IC_SIGNAL_TRUE;
            if (isfinite(signal_data_set_ph_voltage)) {
                signal_data_set_ph_cal_high_V = signal_data_set_ph_voltage;
            }
            break;

        case UI_DWIN_READ_EDIT_SYS_STAGE_ADDRESS:
            signal_data_set_system_stage_data = (u16_val <= 2U) ? (uint8_t)u16_val : 2U;
            signal_data_get_system_stage_data = signal_data_set_system_stage_data;
            break;

        case UI_DWIN_READ_EDIT_BROCCOLI_ADDRESS:     SIGNALS_set_selected_fruit(0U);  break;
        case UI_DWIN_READ_EDIT_STRAWBERRY_ADDRESS:   SIGNALS_set_selected_fruit(1U);  break;
        case UI_DWIN_READ_EDIT_ROCKET_ADDRESS:       SIGNALS_set_selected_fruit(2U);  break;
        case UI_DWIN_READ_EDIT_TOMATO_ADDRESS:       SIGNALS_set_selected_fruit(3U);  break;
        case UI_DWIN_READ_EDIT_LETTUCE_ADDRESS:      SIGNALS_set_selected_fruit(4U);  break;
        case UI_DWIN_READ_EDIT_LEAK_ADDRESS:         SIGNALS_set_selected_fruit(5U);  break;
        case UI_DWIN_READ_EDIT_EGGPLANT_ADDRESS:     SIGNALS_set_selected_fruit(6U);  break;
        case UI_DWIN_READ_EDIT_PEAS_ADDRESS:         SIGNALS_set_selected_fruit(7U);  break;
        case UI_DWIN_READ_EDIT_BR_SPROUTS_ADDRESS:   SIGNALS_set_selected_fruit(8U);  break;
        case UI_DWIN_READ_EDIT_CABBAGE_ADDRESS:      SIGNALS_set_selected_fruit(9U);  break;
        case UI_DWIN_READ_EDIT_CHILLI_ADDRESS:       SIGNALS_set_selected_fruit(10U); break;
        case UI_DWIN_READ_EDIT_SPINACH_ADDRESS:      SIGNALS_set_selected_fruit(11U); break;
        case UI_DWIN_READ_EDIT_PARSLEY_ADDRESS:      SIGNALS_set_selected_fruit(12U); break;
        case UI_DWIN_READ_EDIT_GREEN_BEANS_ADDRESS:  SIGNALS_set_selected_fruit(13U); break;
        case UI_DWIN_READ_EDIT_CUCUMBER_ADDRESS:     SIGNALS_set_selected_fruit(14U); break;
        case UI_DWIN_READ_EDIT_KALE_ADDRESS:         SIGNALS_set_selected_fruit(15U); break;
        case UI_DWIN_READ_EDIT_ASIAN_GREANS_ADDRESS: SIGNALS_set_selected_fruit(16U); break;

        default:
            /* Ignore unrecognized VP addresses safely without corrupting selected fruit */
            break;
    }
}

void check_address(uint8_t *received_data, uint16_t address, uint8_t format)
{
    if (received_data == NULL) {
        return;
    }
    size_t val_offset = 7U;
    if (format == 2U) {
        val_offset = 13U;
    } else if (format == 3U) {
        val_offset = 16U;
    } else if (format == 4U) {
        val_offset = 18U;
    }
    dispatch_dwin_vp_value(address, &received_data[val_offset], 2U);
}

void parse_received_byte(size_t length_of_data, uint8_t *received_data)
{
    if (received_data == NULL || length_of_data == 0U) {
        return;
    }

    size_t offset = 0U;
    while (offset < length_of_data) {
        size_t space = DWIN_RX_STREAM_CAP - s_dwin_rx_len;
        if (space == 0U) {
            memmove(&s_dwin_rx_buf[0], &s_dwin_rx_buf[1], DWIN_RX_STREAM_CAP - 1U);
            s_dwin_rx_len = DWIN_RX_STREAM_CAP - 1U;
            space = 1U;
        }

        size_t chunk = length_of_data - offset;
        if (chunk > space) {
            chunk = space;
        }
        memcpy(&s_dwin_rx_buf[s_dwin_rx_len], &received_data[offset], chunk);
        s_dwin_rx_len += chunk;
        offset += chunk;

        size_t cursor = 0U;
        while (cursor < s_dwin_rx_len) {
            if (s_dwin_rx_buf[cursor] != DWIN_HEADER_BYTE_0) {
                cursor++;
                continue;
            }
            if ((s_dwin_rx_len - cursor) < 2U) {
                /* Wait for 0xA5 second header byte */
                break;
            }
            if (s_dwin_rx_buf[cursor + 1U] != DWIN_HEADER_BYTE_1) {
                cursor++;
                continue;
            }
            if ((s_dwin_rx_len - cursor) < 4U) {
                /* Wait for payload_len + command byte */
                break;
            }

            uint8_t payload_len = s_dwin_rx_buf[cursor + 2U];
            uint8_t cmd = s_dwin_rx_buf[cursor + 3U];

            if (payload_len < 3U || payload_len > 32U ||
                (cmd != UI_DWIN_WRITE_ADDR && cmd != UI_DWIN_READ_ADDR)) {
                /* Invalid DWIN header candidate; advance past 0x5A 0xA5 */
                cursor += 2U;
                continue;
            }

            size_t total_frame_len = (size_t)payload_len + 3U;
            if ((s_dwin_rx_len - cursor) < total_frame_len) {
                /* Split frame across UART reads; wait for remaining bytes */
                break;
            }

            const uint8_t *frame = &s_dwin_rx_buf[cursor];
            if (cmd == UI_DWIN_WRITE_ADDR) {
                /* DWIN Write ACK: 5A A5 03 82 4F 4B */
                if (payload_len == 3U && frame[4] == 0x4FU && frame[5] == 0x4BU) {
                    if (read_write_op_counter > 0U) {
                        read_write_op_counter--;
                    }
                    cursor += total_frame_len;
                } else {
                    cursor += 2U;
                }
            } else {
                /* DWIN VP Upload: 5A A5 <len> 83 <addr_hi> <addr_lo> <word_cnt> <data...> */
                if (payload_len >= 6U) {
                    uint8_t word_cnt = frame[6];
                    size_t expected_payload_len = 4U + (2U * (size_t)word_cnt);
                    if (word_cnt >= 1U && expected_payload_len == (size_t)payload_len) {
                        uint16_t vp_addr = (uint16_t)(((uint16_t)frame[4] << 8) | frame[5]);
                        dispatch_dwin_vp_value(vp_addr, &frame[7], (size_t)word_cnt * 2U);
                        cursor += total_frame_len;
                    } else {
                        cursor += 2U;
                    }
                } else {
                    cursor += 2U;
                }
            }
        }

        if (cursor > 0U) {
            size_t remaining = s_dwin_rx_len - cursor;
            if (remaining > 0U) {
                memmove(&s_dwin_rx_buf[0], &s_dwin_rx_buf[cursor], remaining);
            }
            s_dwin_rx_len = remaining;
        }
    }
}

void UI_INIT(void)
{
    UI_reset_parser();
    read_write_op_counter = 0;
    ui_protocol_counter = 0;

    uart_config_t uart_config = 
    {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB
    };

    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &ui_protocol_uart_queue, 0);
    uart_enable_rx_intr(UART_NUM);

    xTaskCreate(ui_protocol_rx_task, "ui_protocol_rx_task", 1024 * 3, NULL, 23, NULL);
}

void UI_DATA_OUT(void)
{
    if (ui_protocol_counter >= 25U) {
        ui_protocol_counter = 0U;
    }
    uart_write_bytes(UART_NUM, &ui_queue_data[ui_protocol_counter][0], (size_t)(ui_queue_data[ui_protocol_counter][2] + 3U));
    if (read_write_op_counter < 255U) {
        read_write_op_counter++;
    }
    if (ui_protocol_counter < 24U)
    {
        ui_protocol_counter++;
    } else {
        ui_protocol_counter = 0U;
    }
}

void UI_EXE(void)
{
    dataToDisplayBuff(0, signal_data_get_ph_data, ph_conversion_bytes);
    dataToDisplayBuff(1, signal_data_get_ec_data, ec_conversion_bytes);
    dataToDisplayBuff(2, signal_data_get_water_temperature_data, water_temp_conversion_bytes);
    dataToDisplayBuff(3, signal_data_get_water_level_data, distance_conversion_bytes);
    dataToDisplayBuff(4, signal_data_get_water_cap_percentage, tank_cap_percentage_conversion_bytes);
    dataToDisplayBuff(5, signal_data_get_room_temperature_data, room_temp_conversion_bytes);
    dataToDisplayBuff(6, signal_data_get_room_humidity_data, humidity_conversion_bytes);

    ui_queue_data[7][6] = (uint8_t)(signal_data_set_selected_fruit >> 8);
    ui_queue_data[7][7] = (uint8_t)(signal_data_set_selected_fruit & 0x00FF);

    createDwinTextCommand(&signal_data_set_text_selected_fruit[0], &ui_queue_data[8][0]);
    createDwinTextCommand(&signal_data_set_text_led_status[0], &ui_queue_data[9][0]); 
    createDwinTextCommand(&signal_data_set_text_pump_status[0], &ui_queue_data[10][0]); 
    createDwinTextCommand(&signal_data_set_text_sys_status[0], &ui_queue_data[11][0]); 

    ui_queue_data[12][6] = (uint8_t)(signal_data_get_tank_capacity >> 8);
    ui_queue_data[12][7] = (uint8_t)(signal_data_get_tank_capacity & 0x00FF);
  
    ui_queue_data[13][6] = (uint8_t)(signal_data_get_dosing_intensity >> 8);
    ui_queue_data[13][7] = (uint8_t)(signal_data_get_dosing_intensity & 0x00FF);
  
    ui_queue_data[14][6] = (uint8_t)(signal_data_get_ph_concentration_data >> 24);
    ui_queue_data[14][7] = (uint8_t)(signal_data_get_ph_concentration_data >> 16);
    ui_queue_data[14][8] = (uint8_t)(signal_data_get_ph_concentration_data >> 8);
    ui_queue_data[14][9] = (uint8_t)(signal_data_get_ph_concentration_data & 0x000000FF);

    ui_queue_data[15][6] = (uint8_t)(signal_data_get_ec_concentration_data >> 24);
    ui_queue_data[15][7] = (uint8_t)(signal_data_get_ec_concentration_data >> 16);
    ui_queue_data[15][8] = (uint8_t)(signal_data_get_ec_concentration_data >> 8);
    ui_queue_data[15][9] = (uint8_t)(signal_data_get_ec_concentration_data & 0x000000FF);

    ui_queue_data[16][6] = (uint8_t)(signal_data_get_mixing_time >> 8);  
    ui_queue_data[16][7] = (uint8_t)(signal_data_get_mixing_time & 0x00FF);

    if (signal_data_get_rtc_data != NULL) {
        ui_queue_data[17][6] = (uint8_t)(signal_data_get_rtc_data->year >> 8);  
        ui_queue_data[17][7] = (uint8_t)(signal_data_get_rtc_data->year & 0x00FF);

        ui_queue_data[18][6] = 0U;
        ui_queue_data[18][7] = signal_data_get_rtc_data->month;
        ui_queue_data[19][6] = 0U;
        ui_queue_data[19][7] = signal_data_get_rtc_data->dayMonth;
        ui_queue_data[20][6] = 0U;
        ui_queue_data[20][7] = signal_data_get_rtc_data->hour;
        ui_queue_data[21][6] = 0U;
        ui_queue_data[21][7] = signal_data_get_rtc_data->minute;
    }

    dataToDisplayBuff(22, signal_data_set_ec_voltage, ec_voltage_conversion_bytes);
    dataToDisplayBuff(23, signal_data_set_ph_voltage, ph_voltage_conversion_bytes);

    ui_queue_data[24][6] = 0U;
    ui_queue_data[24][7] = signal_data_set_system_stage_data;
}
