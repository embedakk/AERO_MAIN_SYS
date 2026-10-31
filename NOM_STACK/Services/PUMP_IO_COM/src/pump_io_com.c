#include "../inc/pump_io_com.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"
#include "esp_timer.h"

bool busy_flag = false;
esp_err_t internal_uart_driver_error = ESP_OK;
uint8_t pump_protocol_input_byte = 0;
uint8_t pump_protocol_checksum_calc = 0;
uint8_t pump_io_transfer_to_stm_ptr_buffer[PROTOCOL_FRAME_NR][BUFFER_SIZE] = {{0}};
uint8_t pump_io_transfer_to_stm_buffer[FRAME_BUFFER_SIZE] = {0};
uint8_t pump_io_receive_from_stm_ptr_buffer[PROTOCOL_FRAME_NR][RESPONSE_BUFFER_SIZE] = {{0}};
uint8_t pump_io_receive_from_stm_buffer[RESPONSE_FRAME_BUFFER_SIZE] = {0};
pump_protocol_flag_def flag_notification = PASS;
static pump_ctrl_protocol_response_buffer s_parsed_response;
pump_ctrl_protocol_response_buffer *PUMP_IO_COM_response = &s_parsed_response;
uint8_t PUMP_IO_COM_buffer[8] = {0};
uint8_t rx_msg_placement_counter = 0;
int PUMP_IO_COM_rx_len = 0;

pump_protocol_flag_def diagnostics_response[PROTOCOL_FRAME_NR] = {FAILURE};
pump_ctrl_protocol_rcv_buffer send_cmd_data, send_cmd_data_f;

static QueueHandle_t pump_io_com_uart_queue = NULL;
static uart_event_t pump_io_com_task_event;

#define PUMP_IO_RX_STREAM_CAP 256U
static uint8_t s_rx_stream_buf[PUMP_IO_RX_STREAM_CAP];
static size_t s_rx_stream_len = 0;
static bool s_stm32_distance_valid = false;
static int64_t s_last_valid_stm32_rx_us = 0;

float bytesToFloat(uint8_t bytes[4])
{
    union {
        float f;
        uint8_t b[4];
    } u;

    u.b[0] = bytes[3];
    u.b[1] = bytes[2];
    u.b[2] = bytes[1];
    u.b[3] = bytes[0];

    return u.f;
}

static void write_tx_slot_raw(uint8_t slot, uint8_t interface_byte, uint8_t cmd_or_diag)
{
    if (slot >= PROTOCOL_FRAME_NR) {
        return;
    }
    pump_io_transfer_to_stm_ptr_buffer[slot][0] = FRAME_START_BYTE;
    pump_io_transfer_to_stm_ptr_buffer[slot][1] = interface_byte;
    pump_io_transfer_to_stm_ptr_buffer[slot][2] = cmd_or_diag;
    pump_io_transfer_to_stm_ptr_buffer[slot][3] = (uint8_t)(FRAME_START_BYTE + interface_byte + cmd_or_diag);
    pump_io_transfer_to_stm_ptr_buffer[slot][4] = FRAME_END_BYTE;
}

void PUMP_IO_COM_reset_parser(void)
{
    s_rx_stream_len = 0;
    memset(s_rx_stream_buf, 0, sizeof(s_rx_stream_buf));
}

void PUMP_IO_COM_init_state(void)
{
    PUMP_IO_COM_reset_parser();
    s_stm32_distance_valid = false;
    s_last_valid_stm32_rx_us = 0;
    PUMP_IO_COM_response = &s_parsed_response;

    for (uint8_t i = 0; i < PROTOCOL_FRAME_NR; i++) {
        diagnostics_response[i] = FAILURE;
        memset(pump_io_receive_from_stm_ptr_buffer[i], 0, RESPONSE_BUFFER_SIZE);
    }

    /* Populate all 12 TX slots with valid Safe-Off / PASS frames */
    write_tx_slot_raw(0, INTERFACE_EC_PUMPS, COMMAND_EC_UP_OFF);
    write_tx_slot_raw(1, INTERFACE_PH_PUMPS, COMMAND_PH_UP_OFF);
    write_tx_slot_raw(2, INTERFACE_OXY_PUMPS, COMMAND_OFF_OR_DOWN);
    write_tx_slot_raw(3, INTERFACE_MIX_PUMP, COMMAND_OFF_OR_DOWN);
    write_tx_slot_raw(4, INTERFACE_M_IN_VALVE, COMMAND_OFF_OR_DOWN);
    write_tx_slot_raw(5, INTERFACE_M_OUT_VALVE, COMMAND_OFF_OR_DOWN);
    write_tx_slot_raw(6, INTERFACE_P_PUMP, COMMAND_OFF_OR_DOWN);
    write_tx_slot_raw(7, INTERFACE_GROW_LEDS, COMMAND_OFF_OR_DOWN);
    write_tx_slot_raw(8, INTERFACE_PH_SENSOR, PASS);
    write_tx_slot_raw(9, INTERFACE_EC_SENSOR, PASS);
    write_tx_slot_raw(10, INTERFACE_TEMPERATURE_SENSOR, PASS);
    write_tx_slot_raw(11, INTERFACE_DISTANCE_SENSOR, PASS);
}

bool PUMP_IO_COM_is_link_healthy(void)
{
    if (!s_stm32_distance_valid) {
        return false;
    }
    int64_t now_us = esp_timer_get_time();
    int64_t elapsed_us = now_us - s_last_valid_stm32_rx_us;
    return (elapsed_us >= 0 && elapsed_us <= PUMP_IO_COM_LINK_TIMEOUT_US);
}

static void apply_validated_stm32_frame(const uint8_t frame[RESPONSE_BUFFER_SIZE])
{
    uint8_t iface = frame[1];
    uint8_t slot = (uint8_t)(iface & 0x0FU);
    if (slot >= PROTOCOL_FRAME_NR) {
        return;
    }

    memcpy(pump_io_receive_from_stm_ptr_buffer[slot], frame, RESPONSE_BUFFER_SIZE);
    diagnostics_response[slot] = PASS;

    if (iface >= INTERFACE_PH_SENSOR && iface <= INTERFACE_DISTANCE_SENSOR) {
        write_tx_slot_raw(slot, iface, PASS);
    }

    switch (iface) {
    case INTERFACE_PH_SENSOR: {
        uint8_t raw[4] = {frame[2], frame[3], frame[4], frame[5]};
        float val = bytesToFloat(raw);
        if (isfinite(val) && val >= -100.0f && val <= 200.0f) {
            memcpy(pump_io_get_ph_duty_V_byte_data, raw, 4U);
            pump_io_get_ph_duty_V_float_data = val;
        } else {
            diagnostics_response[slot] = FAILURE;
            write_tx_slot_raw(slot, iface, FAILURE);
        }
        break;
    }
    case INTERFACE_EC_SENSOR: {
        uint8_t raw[4] = {frame[2], frame[3], frame[4], frame[5]};
        float val = bytesToFloat(raw);
        if (isfinite(val) && val >= -100.0f && val <= 200.0f) {
            memcpy(pump_io_get_ec_duty_V_byte_data, raw, 4U);
            pump_io_get_ec_duty_V_float_data = val;
        } else {
            diagnostics_response[slot] = FAILURE;
            write_tx_slot_raw(slot, iface, FAILURE);
        }
        break;
    }
    case INTERFACE_TEMPERATURE_SENSOR: {
        uint8_t raw[4] = {frame[2], frame[3], frame[4], frame[5]};
        float val = bytesToFloat(raw);
        if (isfinite(val) && val >= -20.0f && val <= 85.0f) {
            memcpy(pump_io_get_temperature_byte_data, raw, 4U);
            pump_io_get_temperature_float_data = val;
        } else {
            diagnostics_response[slot] = FAILURE;
            write_tx_slot_raw(slot, iface, FAILURE);
        }
        break;
    }
    case INTERFACE_DISTANCE_SENSOR: {
        uint8_t dist = frame[5];
        if (dist >= PUMP_IO_COM_MIN_VALID_DIST_CM && dist <= PUMP_IO_COM_MAX_VALID_DIST_CM) {
            pump_io_get_distance_data = dist;
            s_stm32_distance_valid = true;
            s_last_valid_stm32_rx_us = esp_timer_get_time();
        } else {
            diagnostics_response[slot] = FAILURE;
            write_tx_slot_raw(slot, iface, FAILURE);
        }
        break;
    }
    default:
        break;
    }
}

void PUMP_IO_COM_process_rx_bytes(const uint8_t *data, size_t len)
{
    if (data == NULL || len == 0U) {
        return;
    }

    size_t offset = 0U;
    while (offset < len) {
        size_t space = PUMP_IO_RX_STREAM_CAP - s_rx_stream_len;
        if (space == 0U) {
            /* Drop oldest byte to make room if buffer is full */
            memmove(&s_rx_stream_buf[0], &s_rx_stream_buf[1], PUMP_IO_RX_STREAM_CAP - 1U);
            s_rx_stream_len = PUMP_IO_RX_STREAM_CAP - 1U;
            space = 1U;
        }

        size_t chunk = len - offset;
        if (chunk > space) {
            chunk = space;
        }
        memcpy(&s_rx_stream_buf[s_rx_stream_len], &data[offset], chunk);
        s_rx_stream_len += chunk;
        offset += chunk;

        size_t cursor = 0U;
        while (cursor < s_rx_stream_len) {
            if (s_rx_stream_buf[cursor] != FRAME_START_BYTE) {
                cursor++;
                continue;
            }

            if ((s_rx_stream_len - cursor) < RESPONSE_BUFFER_SIZE) {
                /* Wait for remaining bytes of this 8-byte frame */
                break;
            }

            const uint8_t *cand = &s_rx_stream_buf[cursor];
            pump_ctrl_protocol_response_buffer parsed;
            parsed.start_byte = cand[0];
            parsed.interface_byte = cand[1];
            parsed.response_byte = ((uint32_t)cand[2] << 24) |
                                   ((uint32_t)cand[3] << 16) |
                                   ((uint32_t)cand[4] << 8)  |
                                   ((uint32_t)cand[5]);
            parsed.checksum_byte = cand[6];
            parsed.end_byte = cand[7];

            s_parsed_response = parsed;
            PUMP_IO_COM_response = &s_parsed_response;

            if (PUMP_IO_COM_check_the_buffer_resp_not(&parsed) == PASS) {
                apply_validated_stm32_frame(cand);
                cursor += RESPONSE_BUFFER_SIZE;
            } else {
                /* Record diagnostic failure if frame boundaries & interface matched */
                if (cand[7] == FRAME_END_BYTE &&
                    cand[1] >= INTERFACE_EC_PUMPS &&
                    cand[1] <= INTERFACE_DISTANCE_SENSOR) {
                    uint8_t slot = (uint8_t)(cand[1] & 0x0FU);
                    if (slot < PROTOCOL_FRAME_NR) {
                        diagnostics_response[slot] = FAILURE;
                        if (cand[1] >= INTERFACE_PH_SENSOR) {
                            write_tx_slot_raw(slot, cand[1], FAILURE);
                        }
                    }
                }
                cursor += 1U;
            }
        }

        if (cursor > 0U) {
            size_t remaining = s_rx_stream_len - cursor;
            if (remaining > 0U) {
                memmove(&s_rx_stream_buf[0], &s_rx_stream_buf[cursor], remaining);
            }
            s_rx_stream_len = remaining;
        }
    }
}

void pump_io_com_rx_task(void *arg)
{
    (void)arg;
    uint8_t rx_chunk[128];

    while (1)
    {
        if (xQueueReceive(pump_io_com_uart_queue, &pump_io_com_task_event, portMAX_DELAY)) 
        {
            if (pump_io_com_task_event.type == UART_FIFO_OVF ||
                pump_io_com_task_event.type == UART_BUFFER_FULL)
            {
                uart_flush_input(UART_NUM_2);
                xQueueReset(pump_io_com_uart_queue);
                SIGNALS_lock();
                PUMP_IO_COM_reset_parser();
                SIGNALS_unlock();
            }
            else if (pump_io_com_task_event.type == UART_DATA) 
            {
                size_t buffered_len = 0;
                uart_get_buffered_data_len(UART_NUM_2, &buffered_len);
                while (buffered_len > 0U) {
                    size_t to_read = (buffered_len > sizeof(rx_chunk)) ? sizeof(rx_chunk) : buffered_len;
                    int read_bytes = uart_read_bytes(UART_NUM_2, rx_chunk, (uint32_t)to_read, 0);
                    if (read_bytes <= 0) {
                        break;
                    }
                    SIGNALS_lock();
                    PUMP_IO_COM_process_rx_bytes(rx_chunk, (size_t)read_bytes);
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

void PUMP_IO_COM_print_pump_response(const pump_ctrl_protocol_response_buffer *msg)
{
    (void)msg;
#ifdef PUMP_IO_COM_LOGS
    if (msg) {
        ESP_LOGI(PUMP_IO_COM_TAG, "Pump Int Control Response ...");
        ESP_LOGI(PUMP_IO_COM_TAG, "start byte : 0x%02X ", msg->start_byte);
        ESP_LOGI(PUMP_IO_COM_TAG, "interface  : 0x%02X ", msg->interface_byte);
        ESP_LOGI(PUMP_IO_COM_TAG, "response   : 0x%08X ", msg->response_byte);
        ESP_LOGI(PUMP_IO_COM_TAG, "checksum   : 0x%02X ", msg->checksum_byte);
        ESP_LOGI(PUMP_IO_COM_TAG, "end byte   : 0x%02X ", msg->end_byte);
    }
#endif
}

void PUMP_IO_COM_print_command(const pump_ctrl_protocol_rcv_buffer *msg)
{
    (void)msg;
#ifdef PUMP_IO_COM_LOGS
    if (msg) {
        ESP_LOGI(PUMP_IO_COM_TAG, "Pump Int Control Command ...");
        ESP_LOGI(PUMP_IO_COM_TAG, "start byte : 0x%02X", msg->start_byte);
        ESP_LOGI(PUMP_IO_COM_TAG, "interface  : 0x%02X", msg->interface_byte);
        ESP_LOGI(PUMP_IO_COM_TAG, "command    : 0x%02X", msg->command_byte);
        ESP_LOGI(PUMP_IO_COM_TAG, "checksum   : 0x%02X", msg->checksum_byte);
        ESP_LOGI(PUMP_IO_COM_TAG, "end byte   : 0x%02X", msg->end_byte);
    }
#endif
}

void PUMP_IO_COM_INIT_the_main_pump_ctrl(void)
{
    PUMP_IO_COM_init_state();

    const uart_config_t uart_config = 
    {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    internal_uart_driver_error = uart_param_config(UART_NUM_2, &uart_config);
    internal_uart_driver_error = uart_set_pin(UART_NUM_2, PUMP_IO_COM_TXD_PIN, PUMP_IO_COM_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    internal_uart_driver_error = uart_driver_install(UART_NUM_2, 1024 * 2, 1024 * 2, 20, &pump_io_com_uart_queue, 0);
    uart_enable_rx_intr(UART_NUM_2);
    xTaskCreate(pump_io_com_rx_task, "pump_io_com_rx_task", 1024 * 4, NULL, 25, NULL);
}

pump_protocol_flag_def PUMP_IO_COM_check_len(uint8_t size_of_buffer)
{
    if (size_of_buffer != RESPONSE_BUFFER_SIZE)
    {
        return FAILURE;
    }
    return PASS;
}

pump_protocol_flag_def PUMP_IO_COM_check_start_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer)
{
    if (!input_data_buffer || input_data_buffer->start_byte != FRAME_START_BYTE)
    {
        return FAILURE;
    }
    return PASS;
}

pump_protocol_flag_def PUMP_IO_COM_check_end_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer)
{
    if (!input_data_buffer || input_data_buffer->end_byte != FRAME_END_BYTE)
    {
        return FAILURE;
    }
    return PASS;
}

pump_protocol_flag_def PUMP_IO_COM_check_interface_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer)
{
    if (!input_data_buffer) {
        return FAILURE;
    }
    if (input_data_buffer->interface_byte < INTERFACE_EC_PUMPS ||
        input_data_buffer->interface_byte > INTERFACE_DISTANCE_SENSOR)
    {
        return FAILURE;
    }
    return PASS;
}

pump_protocol_flag_def PUMP_IO_COM_check_response_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer)
{
    if (!input_data_buffer) {
        return FAILURE;
    }
    if (input_data_buffer->interface_byte < INTERFACE_PH_SENSOR)
    {
        if (input_data_buffer->response_byte != TRANSFER_COMP_SUCC)
        {
            return FAILURE;                
        }
    }
    return PASS;
}

pump_protocol_flag_def PUMP_IO_COM_check_checksum_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer)
{
    if (!input_data_buffer) {
        return FAILURE;
    }
    uint32_t r = input_data_buffer->response_byte;
    uint8_t csum_legacy = (uint8_t)(input_data_buffer->start_byte + input_data_buffer->interface_byte + r);
    uint8_t csum_full   = (uint8_t)(input_data_buffer->start_byte + input_data_buffer->interface_byte +
                                    ((r >> 24) & 0xFFU) + ((r >> 16) & 0xFFU) + ((r >> 8) & 0xFFU) + (r & 0xFFU));
    pump_protocol_checksum_calc = csum_legacy;
    if (input_data_buffer->checksum_byte != csum_legacy &&
        input_data_buffer->checksum_byte != csum_full)
    {
        return FAILURE;
    }
    return PASS;
}

pump_protocol_flag_def PUMP_IO_COM_check_the_buffer_resp_not(const pump_ctrl_protocol_response_buffer *input_data_buffer)
{
    if (!input_data_buffer) {
        return FAILURE;
    }

    flag_notification = PUMP_IO_COM_check_start_byte(input_data_buffer);
    if (flag_notification == FAILURE)
    {
        return FAILURE;
    }
  
    flag_notification = PUMP_IO_COM_check_end_byte(input_data_buffer);
    if (flag_notification == FAILURE)
    {
        return FAILURE;
    }
  
    flag_notification = PUMP_IO_COM_check_interface_byte(input_data_buffer);
    if (flag_notification == FAILURE)
    {
        return FAILURE;
    }
  
    flag_notification = PUMP_IO_COM_check_response_byte(input_data_buffer);
    if (flag_notification == FAILURE)
    {
        return FAILURE;
    }
  
    flag_notification = PUMP_IO_COM_check_checksum_byte(input_data_buffer);
    if (flag_notification == FAILURE)
    {
        return FAILURE;
    }

    return PASS;
}

void PUMP_IO_COM_send_pump_command(const pump_protocol_interface_byteDef interface, const pump_protocol_command_byteDef command)
{
    if (interface < INTERFACE_EC_PUMPS || interface > INTERFACE_DISTANCE_SENSOR) {
        return;
    }
    uint8_t slot = (uint8_t)(interface & 0x0FU);
    write_tx_slot_raw(slot, (uint8_t)interface, (uint8_t)command);
}

void PUMP_IO_COM_EXE(void)
{
    for (rx_msg_placement_counter = 0; rx_msg_placement_counter < PROTOCOL_FRAME_NR; rx_msg_placement_counter++)
    {
        pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE)]     = pump_io_transfer_to_stm_ptr_buffer[rx_msg_placement_counter][0];
        pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE) + 1] = pump_io_transfer_to_stm_ptr_buffer[rx_msg_placement_counter][1];
        pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE) + 2] = pump_io_transfer_to_stm_ptr_buffer[rx_msg_placement_counter][2];
        pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE) + 3] = pump_io_transfer_to_stm_ptr_buffer[rx_msg_placement_counter][3];
        pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE) + 4] = pump_io_transfer_to_stm_ptr_buffer[rx_msg_placement_counter][4];
    }
    uart_write_bytes(UART_NUM_2, &pump_io_transfer_to_stm_buffer[0], FRAME_BUFFER_SIZE);
}

bool PUMP_IO_COM_check_busy_flag(void)
{
    return false;
}
