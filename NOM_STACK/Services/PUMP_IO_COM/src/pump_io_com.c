#include "../inc/pump_io_com.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"
static QueueHandle_t pump_io_com_uart_queue;
uart_event_t pump_io_com_handler_event;
uart_event_t pump_io_com_task_event;
BaseType_t xPumpIoComPriorityTaskWoken = pdFALSE;
uint8_t *pump_io_com_dma_buffer;
size_t pump_io_com_len;

float bytesToFloat(uint8_t bytes[4])
{
    union {
        float f;
        uint8_t b[4];
    } u;

    // Assuming little-endian byte order (adjust if needed)
    // u.b[0] = bytes[0];
    // u.b[1] = bytes[1];
    // u.b[2] = bytes[2];
    // u.b[3] = bytes[3];
    u.b[0] = bytes[3];
    u.b[1] = bytes[2];
    u.b[2] = bytes[1];
    u.b[3] = bytes[0];

    return u.f;
}

void pump_io_com_rx_task(void *arg)
{
    PUMP_IO_COM_response = (pump_ctrl_protocol_response_buffer *)&PUMP_IO_COM_buffer[0];
    while (1)
    {
 
        if (xQueueReceive(pump_io_com_uart_queue, &pump_io_com_task_event, portMAX_DELAY)) 
        {
            // if(pump_io_com_task_event.type == UART_FIFO_OVF)
            // {
            //     ESP_LOGW("Debug_IO", "UART FIFO overflow");
            //     uart_flush_input(UART_NUM_2);
            //     xQueueReset(pump_io_com_uart_queue);
            // }else if (pump_io_com_task_event.type == UART_BUFFER_FULL)
            // {
            //     ESP_LOGW("Debug_IO", "UART RX buffer full");
            //     uart_flush_input(UART_NUM_2);
            //     xQueueReset(pump_io_com_uart_queue);
            // }else if (pump_io_com_task_event.type == UART_FRAME_ERR)
            // {
            //     ESP_LOGW("Debug_IO", "UART frame error");
            // }else if (pump_io_com_task_event.type == UART_PARITY_ERR)
            // {
            //     ESP_LOGW("Debug_IO", "UART parity error");
            // }else if (pump_io_com_task_event.type == UART_BREAK)
            // {
            //     ESP_LOGW("Debug_IO", "UART_BREAK");
            // }else if (pump_io_com_task_event.type == UART_DATA_BREAK)
            // {
            //     ESP_LOGW("Debug_IO", "UART_DATA_BREAK");
            // }else if (pump_io_com_task_event.type == UART_PATTERN_DET)
            // {
            //     ESP_LOGW("Debug_IO", "UART_PATTERN_DET");
            // }else if (pump_io_com_task_event.type == UART_EVENT_MAX)
            // {
            //     ESP_LOGW("Debug_IO", "UART_EVENT_MAX");
            // }
            
            if (pump_io_com_task_event.type == UART_DATA) 
            {

                // ✅ Zero-Copy: Get DMA buffer pointer (No CPU copying!)
                uart_get_buffered_data_len(UART_NUM_2, &pump_io_com_len);
                // printf("pump io com len = %d\r\n", pump_io_com_len);
                pump_io_com_dma_buffer = (uint8_t *)malloc(pump_io_com_len + 32);

                if (pump_io_com_dma_buffer) 
                {
                    uart_read_bytes(UART_NUM_2, pump_io_com_dma_buffer, pump_io_com_len, 0);
                    // ESP_LOGI("RX IO COM ", "Received (DMA Buffer):"); // %.*s", len, dma_buffer);
                    // printf("pump io com len = %d\r\n", pump_io_com_len);
                    // if(pump_io_com_len == 96)
                    // {
                        // for(uint8_t loop_cntr = 0; loop_cntr < 96; loop_cntr++)
                        // {
                        //     if(loop_cntr == 8 || loop_cntr == 16 || loop_cntr == 24 || loop_cntr == 32 || loop_cntr == 40 || loop_cntr == 48 || loop_cntr == 56 || loop_cntr == 64 || loop_cntr == 72 || loop_cntr == 80 || loop_cntr == 88)
                        //     {
                        //         printf("\r\n");
                        //     }
                        //     printf("0x%02X, ", pump_io_com_dma_buffer[loop_cntr]);
                        // }
                        // printf("\r\n");
                    // }
                    if(pump_io_com_len == 96)
                    {
                        // for(uint8_t loop_cntr = 0; loop_cntr < 96; loop_cntr++)
                        // {
                        //     if(loop_cntr == 8 || loop_cntr == 16 || loop_cntr == 24 || loop_cntr == 32 || loop_cntr == 40 || loop_cntr == 48 || loop_cntr == 56 || loop_cntr == 64 || loop_cntr == 72 || loop_cntr == 80 || loop_cntr == 88)
                        //     {
                        //         printf("\r\n");
                        //     }
                        //     printf("0x%02X, ", pump_io_com_dma_buffer[loop_cntr]);
                        // }
                        // printf("\r\n");
                        for(rx_msg_placement_counter = 0; rx_msg_placement_counter < PROTOCOL_FRAME_NR; rx_msg_placement_counter++)
                        {
                            pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][0] = pump_io_com_dma_buffer[rx_msg_placement_counter * RESPONSE_BUFFER_SIZE];
                            pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][1] = pump_io_com_dma_buffer[(rx_msg_placement_counter * RESPONSE_BUFFER_SIZE) + 1];
                            pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][2] = pump_io_com_dma_buffer[(rx_msg_placement_counter * RESPONSE_BUFFER_SIZE) + 2];
                            pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][3] = pump_io_com_dma_buffer[(rx_msg_placement_counter * RESPONSE_BUFFER_SIZE) + 3];
                            pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][4] = pump_io_com_dma_buffer[(rx_msg_placement_counter * RESPONSE_BUFFER_SIZE) + 4];
                            pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][5] = pump_io_com_dma_buffer[(rx_msg_placement_counter * RESPONSE_BUFFER_SIZE) + 5];
                            pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][6] = pump_io_com_dma_buffer[(rx_msg_placement_counter * RESPONSE_BUFFER_SIZE) + 6];
                            pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][7] = pump_io_com_dma_buffer[(rx_msg_placement_counter * RESPONSE_BUFFER_SIZE) + 7];
                            // printf("Buffer Data 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n",
                            //     pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][0],
                            //     pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][1],
                            //     pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][2],
                            //     pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][3],
                            //     pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][4],
                            //     pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][5],
                            //     pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][6],
                            //     pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][7]);
                            // PUMP_IO_COM_response = (pump_ctrl_protocol_response_buffer *)&pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][0];
                            PUMP_IO_COM_response->start_byte = pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][0];
                            PUMP_IO_COM_response->interface_byte = pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][1];
                            PUMP_IO_COM_response->response_byte = (pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][2] << 24) |
                            (pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][3] << 16) | (pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][4] << 8) |
                            (pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][5]);
                            PUMP_IO_COM_response->checksum_byte = (pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][6]);
                            PUMP_IO_COM_response->end_byte = (pump_io_receive_from_stm_ptr_buffer[rx_msg_placement_counter][7]);
                            // PUMP_IO_COM_print_pump_response(PUMP_IO_COM_response);
                            // printf("ptr data 0x%02X 0x%02X 0x%08X 0x%02X 0x%02X \r\n",
                            //     PUMP_IO_COM_response->start_byte,PUMP_IO_COM_response->interface_byte, PUMP_IO_COM_response->response_byte,
                            //     PUMP_IO_COM_response->checksum_byte, PUMP_IO_COM_response->end_byte);
                            diagnostics_response[rx_msg_placement_counter] = PUMP_IO_COM_check_the_buffer_resp_not(PUMP_IO_COM_response);
                            
                            //here diagnostics
                            switch (rx_msg_placement_counter)
                            {
                            case (INTERFACE_PH_SENSOR & 0x0F):
                                {
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_PH_SENSOR & 0x0F][0] = FRAME_START_BYTE;
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_PH_SENSOR & 0x0F][1] = INTERFACE_PH_SENSOR;
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_PH_SENSOR & 0x0F][2] = diagnostics_response[rx_msg_placement_counter];
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_PH_SENSOR & 0x0F][3] = (uint8_t)
                                    (pump_io_transfer_to_stm_ptr_buffer[INTERFACE_PH_SENSOR & 0x0F][0] + pump_io_transfer_to_stm_ptr_buffer[INTERFACE_PH_SENSOR & 0x0F][1] +
                                        pump_io_transfer_to_stm_ptr_buffer[INTERFACE_PH_SENSOR & 0x0F][2]);
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_PH_SENSOR & 0x0F][4] = FRAME_END_BYTE;
                                }
                                break;
                            case (INTERFACE_EC_SENSOR & 0x0F):
                                {
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_EC_SENSOR & 0x0F][0] = FRAME_START_BYTE;
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_EC_SENSOR & 0x0F][1] = INTERFACE_EC_SENSOR;
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_EC_SENSOR & 0x0F][2] = diagnostics_response[rx_msg_placement_counter];
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_EC_SENSOR & 0x0F][3] = (uint8_t)
                                    (pump_io_transfer_to_stm_ptr_buffer[INTERFACE_EC_SENSOR & 0x0F][0] + pump_io_transfer_to_stm_ptr_buffer[INTERFACE_EC_SENSOR & 0x0F][1] +
                                        pump_io_transfer_to_stm_ptr_buffer[INTERFACE_EC_SENSOR & 0x0F][2]);
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_EC_SENSOR & 0x0F][4] = FRAME_END_BYTE;
                                }
                                break;
                            case (INTERFACE_TEMPERATURE_SENSOR & 0x0F):
                                {
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_TEMPERATURE_SENSOR & 0x0F][0] = FRAME_START_BYTE;
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_TEMPERATURE_SENSOR & 0x0F][1] = INTERFACE_TEMPERATURE_SENSOR;
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_TEMPERATURE_SENSOR & 0x0F][2] = diagnostics_response[rx_msg_placement_counter];
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_TEMPERATURE_SENSOR & 0x0F][3] = (uint8_t)
                                    (pump_io_transfer_to_stm_ptr_buffer[INTERFACE_TEMPERATURE_SENSOR & 0x0F][0] + pump_io_transfer_to_stm_ptr_buffer[INTERFACE_TEMPERATURE_SENSOR & 0x0F][1] +
                                        pump_io_transfer_to_stm_ptr_buffer[INTERFACE_TEMPERATURE_SENSOR & 0x0F][2]);
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_TEMPERATURE_SENSOR & 0x0F][4] = FRAME_END_BYTE;
                                }
                                break;
                            case (INTERFACE_DISTANCE_SENSOR & 0x0F):
                                {
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_DISTANCE_SENSOR & 0x0F][0] = FRAME_START_BYTE;
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_DISTANCE_SENSOR & 0x0F][1] = INTERFACE_DISTANCE_SENSOR;
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_DISTANCE_SENSOR & 0x0F][2] = diagnostics_response[rx_msg_placement_counter];
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_DISTANCE_SENSOR & 0x0F][3] = (uint8_t)
                                    (pump_io_transfer_to_stm_ptr_buffer[INTERFACE_DISTANCE_SENSOR & 0x0F][0] + pump_io_transfer_to_stm_ptr_buffer[INTERFACE_DISTANCE_SENSOR & 0x0F][1] +
                                        pump_io_transfer_to_stm_ptr_buffer[INTERFACE_DISTANCE_SENSOR & 0x0F][2]);
                                    pump_io_transfer_to_stm_ptr_buffer[INTERFACE_DISTANCE_SENSOR & 0x0F][4] = FRAME_END_BYTE;
                                }
                                break;
                            
                            default:
                                break;
                            }
                        }
                        //put the raw data's
                        //ph data
                        pump_io_get_ph_duty_V_byte_data[0]      = pump_io_receive_from_stm_ptr_buffer[INTERFACE_PH_SENSOR & 0x0F][2];
                        pump_io_get_ph_duty_V_byte_data[1]      = pump_io_receive_from_stm_ptr_buffer[INTERFACE_PH_SENSOR & 0x0F][3];
                        pump_io_get_ph_duty_V_byte_data[2]      = pump_io_receive_from_stm_ptr_buffer[INTERFACE_PH_SENSOR & 0x0F][4];
                        pump_io_get_ph_duty_V_byte_data[3]      = pump_io_receive_from_stm_ptr_buffer[INTERFACE_PH_SENSOR & 0x0F][5];
                        pump_io_get_ph_duty_V_float_data = bytesToFloat(pump_io_get_ph_duty_V_byte_data);
                        //ec data       
                        // pump_io_get_ec_duty_V_byte_data      
                        pump_io_get_ec_duty_V_byte_data[0]      = pump_io_receive_from_stm_ptr_buffer[INTERFACE_EC_SENSOR & 0x0F][2];
                        pump_io_get_ec_duty_V_byte_data[1]      = pump_io_receive_from_stm_ptr_buffer[INTERFACE_EC_SENSOR & 0x0F][3];
                        pump_io_get_ec_duty_V_byte_data[2]      = pump_io_receive_from_stm_ptr_buffer[INTERFACE_EC_SENSOR & 0x0F][4];
                        pump_io_get_ec_duty_V_byte_data[3]      = pump_io_receive_from_stm_ptr_buffer[INTERFACE_EC_SENSOR & 0x0F][5];
                        pump_io_get_ec_duty_V_float_data = bytesToFloat(pump_io_get_ec_duty_V_byte_data);
                        //distance data
                        pump_io_get_distance_data               = pump_io_receive_from_stm_ptr_buffer[INTERFACE_DISTANCE_SENSOR & 0x0F][5];
                        // printf("pump_io_get_distance_data 0x%d \r\n", pump_io_get_distance_data);
                         //temp data
                        pump_io_get_temperature_byte_data[0]    = pump_io_receive_from_stm_ptr_buffer[INTERFACE_TEMPERATURE_SENSOR & 0x0F][2];
                        pump_io_get_temperature_byte_data[1]    = pump_io_receive_from_stm_ptr_buffer[INTERFACE_TEMPERATURE_SENSOR & 0x0F][3];
                        pump_io_get_temperature_byte_data[2]    = pump_io_receive_from_stm_ptr_buffer[INTERFACE_TEMPERATURE_SENSOR & 0x0F][4];
                        pump_io_get_temperature_byte_data[3]    = pump_io_receive_from_stm_ptr_buffer[INTERFACE_TEMPERATURE_SENSOR & 0x0F][5];
                        // printf("pump_io_get_temperature_byte_data 0x%02X 0x%02X 0x%02X 0x%02X \r\n", pump_io_get_temperature_byte_data[3], pump_io_get_temperature_byte_data[2], pump_io_get_temperature_byte_data[1], pump_io_get_temperature_byte_data[0]);
                        pump_io_get_temperature_float_data= bytesToFloat(pump_io_get_temperature_byte_data);              
                        // printf("pump_io_get_temperature_float_data %.2f \r\n", pump_io_get_temperature_float_data);
                    }

                    //put here the touch logic
                    // ui_protocol_input = &dma_buffer;
                    // parse_received_byte(len, (uint8_t *)&dma_buffer[0]);
                    // printf("Diagnostic PUMP IO %d %d %d %d %d %d %d %d %d %d %d %d \r\n", diagnostics_response[0], diagnostics_response[1], 
                    //     diagnostics_response[2], diagnostics_response[3], diagnostics_response[4], diagnostics_response[5], diagnostics_response[6], diagnostics_response[7], diagnostics_response[8], 
                    //     diagnostics_response[9], diagnostics_response[10], diagnostics_response[11]);
                    free(pump_io_com_dma_buffer);  // Free buffer after processing
                }
            }
        }
        // vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// ISR Handler (Handles UART Events)
void IRAM_ATTR pump_io_com__uart_isr_handler(void *arg) 
{


    if (xQueueReceiveFromISR(pump_io_com_uart_queue, &pump_io_com_handler_event, &xPumpIoComPriorityTaskWoken))
    {
        if (pump_io_com_handler_event.type == UART_DATA) 
        {
            // ESP_LOGI(PUMP_IO_COM_TAG, "ISR: New UART Data, Length: %d", pump_io_com_handler_event.size);
        }
    }
    portYIELD_FROM_ISR(xPumpIoComPriorityTaskWoken);
}

void PUMP_IO_COM_print_pump_response(const pump_ctrl_protocol_response_buffer *msg)
{
#ifdef PUMP_IO_COM_LOGS
    ESP_LOGI(PUMP_IO_COM_TAG, "Pump Int Control Response ...");
    ESP_LOGI(PUMP_IO_COM_TAG, "start byte : 0x%02X ", msg->start_byte);
    ESP_LOGI(PUMP_IO_COM_TAG, "interface  : 0x%02X ", msg->interface_byte);
    ESP_LOGI(PUMP_IO_COM_TAG, "response   : 0x%08X ", msg->response_byte);
    ESP_LOGI(PUMP_IO_COM_TAG, "checksum   : 0x%02X ", msg->checksum_byte);
    ESP_LOGI(PUMP_IO_COM_TAG, "end byte   : 0x%02X ", msg->end_byte);
#endif
    return;
}

void PUMP_IO_COM_print_command(const pump_ctrl_protocol_rcv_buffer *msg)
{
#ifdef PUMP_IO_COM_LOGS
    ESP_LOGI(PUMP_IO_COM_TAG, "Pump Int Control Command ...");
    ESP_LOGI(PUMP_IO_COM_TAG, "start byte : 0x%02X", msg->start_byte);
    ESP_LOGI(PUMP_IO_COM_TAG, "interface  : 0x%02X", msg->interface_byte);
    ESP_LOGI(PUMP_IO_COM_TAG, "command    : 0x%02X", msg->command_byte);
    ESP_LOGI(PUMP_IO_COM_TAG, "checksum   : 0x%02X", msg->checksum_byte);
    ESP_LOGI(PUMP_IO_COM_TAG, "end byte   : 0x%02X", msg->end_byte);
#endif
    return; 
}

void PUMP_IO_COM_INIT_the_main_pump_ctrl(void)
{
    const uart_config_t uart_config = 
    {
        .baud_rate = 115200, //9600, changed from 9600 to 115200
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        // .source_clk = UART_SCLK_APB,
        .source_clk = UART_SCLK_APB,
    };
    internal_uart_driver_error = uart_param_config(UART_NUM_2, &uart_config);
    if(internal_uart_driver_error == ESP_FAIL)
    {
        //add diagnostic error
#ifdef PUMP_IO_COM_LOGS
        ESP_LOGE(PUMP_IO_COM_TAG, "uart_param_config ==>> UART ESP FAILURE !!!");
#endif
    }else
    {
#ifdef PUMP_IO_COM_LOGS
        ESP_LOGI(PUMP_IO_COM_TAG, "uart_param_config ==>> UART ESP OKK ...");
#endif    
    }
    
    internal_uart_driver_error = uart_set_pin(UART_NUM_2, PUMP_IO_COM_TXD_PIN, PUMP_IO_COM_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if(internal_uart_driver_error == ESP_FAIL)
    {
        //add diagnostic error
#ifdef PUMP_IO_COM_LOGS
        ESP_LOGE(PUMP_IO_COM_TAG, "uart_set_pin ==>> UART ESP FAILURE !!!");
#endif    
    }else
    {
#ifdef PUMP_IO_COM_LOGS
        ESP_LOGI(PUMP_IO_COM_TAG, "uart_set_pin ==>> UART ESP OKK ...");
#endif 
    }
    // We won't use a buffer for sending data.
    internal_uart_driver_error = uart_driver_install(UART_NUM_2, 1024 * 2, 1024 * 2, 20, &pump_io_com_uart_queue, 0);
    if(internal_uart_driver_error == ESP_FAIL)
    {
        //add diagnostic error
#ifdef PUMP_IO_COM_LOGS
        ESP_LOGE(PUMP_IO_COM_TAG, "uart_driver_install ==>> UART ESP FAILURE !!!");
#endif
    }else
    {
#ifdef PUMP_IO_COM_LOGS
        ESP_LOGI(PUMP_IO_COM_TAG, "uart_driver_install ==>> UART ESP OKK ...");
#endif
    }
    uart_enable_rx_intr(UART_NUM_2);
    xTaskCreate(pump_io_com_rx_task, "pump_io_com_rx_task", 1024*4, NULL, 25, NULL);
    return;
}

pump_protocol_flag_def PUMP_IO_COM_check_len(uint8_t size_of_buffer)
{
    if(size_of_buffer != RESPONSE_BUFFER_SIZE)
    {
        //add diagnostic here
        return FAILURE;
    }
    return PASS;
}

pump_protocol_flag_def PUMP_IO_COM_check_start_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer)
{
    if(input_data_buffer->start_byte != FRAME_START_BYTE)
    {
        //add diagnostic here
        return FAILURE;
    }
    return PASS;
}

pump_protocol_flag_def PUMP_IO_COM_check_end_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer)
{
    if(input_data_buffer->end_byte != FRAME_END_BYTE)
    {
        //add diagnostic here
        return FAILURE;
    }
    return PASS;
}

pump_protocol_flag_def PUMP_IO_COM_check_interface_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer)
{
    if((input_data_buffer->interface_byte != INTERFACE_EC_PUMPS) & (input_data_buffer->interface_byte != INTERFACE_PH_PUMPS) & (input_data_buffer->interface_byte != INTERFACE_OXY_PUMPS) &
    (input_data_buffer->interface_byte != INTERFACE_MIX_PUMP) & (input_data_buffer->interface_byte != INTERFACE_M_IN_VALVE) & (input_data_buffer->interface_byte != INTERFACE_M_OUT_VALVE) &
    (input_data_buffer->interface_byte != INTERFACE_P_PUMP)   & (input_data_buffer->interface_byte != INTERFACE_GROW_LEDS) &
    (input_data_buffer->interface_byte != INTERFACE_PH_SENSOR)   & (input_data_buffer->interface_byte != INTERFACE_EC_SENSOR) &
    (input_data_buffer->interface_byte != INTERFACE_TEMPERATURE_SENSOR)   & (input_data_buffer->interface_byte != INTERFACE_DISTANCE_SENSOR))
    {
        //add diagnostic here
        return FAILURE;
    }
    return PASS;
}

pump_protocol_flag_def PUMP_IO_COM_check_response_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer)
{
    if(input_data_buffer->interface_byte < INTERFACE_PH_SENSOR)
    {
        if(input_data_buffer->response_byte != TRANSFER_COMP_SUCC)
        {
            //add diagnostic here
            return FAILURE;                
        }
    }
    return PASS;
}

pump_protocol_flag_def PUMP_IO_COM_check_checksum_byte(const pump_ctrl_protocol_response_buffer *input_data_buffer)
{
    pump_protocol_checksum_calc = (uint8_t)(input_data_buffer->start_byte + input_data_buffer->interface_byte + input_data_buffer->response_byte);
    // ESP_LOGI(PUMP_TAG, "Response checksum value ==>> 0x%02X", pump_protocol_checksum_calc);
    if(input_data_buffer->checksum_byte != pump_protocol_checksum_calc)
    {
        //add diagnostic here
        return FAILURE;
    }
    return PASS;
}

pump_protocol_flag_def PUMP_IO_COM_check_the_buffer_resp_not(const pump_ctrl_protocol_response_buffer *input_data_buffer)
{
    // check start byte of the input buffer
    flag_notification = PUMP_IO_COM_check_start_byte(input_data_buffer);
    if(flag_notification == FAILURE)
    {
#ifdef PUMP_IO_COM_LOGS
      ESP_LOGE(PUMP_IO_COM_TAG, "Start Byte Failure!!! ==>> 0x%02X", input_data_buffer->start_byte);
#endif
      return FAILURE;
    }
  
    // check end byte of the input buffer
    flag_notification = PUMP_IO_COM_check_end_byte(input_data_buffer);
    if(flag_notification == FAILURE)
    {
#ifdef PUMP_IO_COM_LOGS
      ESP_LOGE(PUMP_IO_COM_TAG, "End Byte Failure!!! ==>> 0x%02X", input_data_buffer->end_byte);
#endif
      return FAILURE;
    }
  
    // check the interface byte of the input buffer
    flag_notification = PUMP_IO_COM_check_interface_byte(input_data_buffer);
    if(flag_notification == FAILURE)
    {
#ifdef PUMP_IO_COM_LOGS
      ESP_LOGE(PUMP_IO_COM_TAG, "Interface Byte Failure!!! ==>> 0x%02X", input_data_buffer->interface_byte);
#endif
      return FAILURE;
    }
  
    // check the command byte of the input buffer
    flag_notification = PUMP_IO_COM_check_response_byte(input_data_buffer);
    if(flag_notification == FAILURE)
    {
#ifdef PUMP_IO_COM_LOGS
      ESP_LOGE(PUMP_IO_COM_TAG, "Response Byte Failure!!! ==>> 0x%02X", input_data_buffer->response_byte);
#endif
      return FAILURE;
    }
  
    // check the checksum byte of the input buffer
    flag_notification = PUMP_IO_COM_check_checksum_byte(input_data_buffer);
    if(flag_notification == FAILURE)
    {
#ifdef PUMP_IO_COM_LOGS
      ESP_LOGE(PUMP_IO_COM_TAG, "Checksum Failure!!! ==>> 0x%02X", input_data_buffer->checksum_byte);
#endif
      return FAILURE;
    }
#ifdef PUMP_IO_COM_LOGS  
    ESP_LOGD(PUMP_IO_COM_TAG, "Command Execution Completed Successfully...");
#endif
    return PASS;
}

void PUMP_IO_COM_send_pump_command(const pump_protocol_interface_byteDef interface, const pump_protocol_command_byteDef command)
{
    //all commands execute in one cycle with loop
    // busy_flag = true;
    /////////////////////////////////////////
    // send_cmd_data_f.start_byte = FRAME_START_BYTE;
    // send_cmd_data_f.interface_byte = interface;
    // send_cmd_data_f.command_byte = command;
    // send_cmd_data_f.end_byte = FRAME_END_BYTE;
    // send_cmd_data_f.checksum_byte = (uint8_t)(send_cmd_data_f.start_byte + send_cmd_data_f.interface_byte + send_cmd_data_f.command_byte);
    // PUMP_IO_COM_print_command(&send_cmd_data_f);
    // memcpy(PUMP_IO_COM_buffer, &send_cmd_data_f, 5);
    // uart_write_bytes(UART_NUM_2, (char *)PUMP_IO_COM_buffer, 5);
    // PUMP_IO_COM_rx_len = uart_read_bytes(UART_NUM_2, PUMP_IO_COM_buffer, 5, 2/portTICK_RATE_MS);
    // if(PUMP_IO_COM_rx_len > 0)
    // {
    //     PUMP_IO_COM_buffer[PUMP_IO_COM_rx_len] = 0;
    //     PUMP_IO_COM_response = (pump_ctrl_protocol_response_buffer *)&PUMP_IO_COM_buffer[0];
    //     PUMP_IO_COM_print_pump_response(PUMP_IO_COM_response);
    //     PUMP_IO_COM_check_the_buffer_resp_not(PUMP_IO_COM_response);
    // }
    pump_io_transfer_to_stm_ptr_buffer[interface & 0x0F][0] = FRAME_START_BYTE;
    pump_io_transfer_to_stm_ptr_buffer[interface & 0x0F][1] = interface;
    pump_io_transfer_to_stm_ptr_buffer[interface & 0x0F][2] = command;
    pump_io_transfer_to_stm_ptr_buffer[interface & 0x0F][3] = (uint8_t)
    (pump_io_transfer_to_stm_ptr_buffer[interface & 0x0F][0] + pump_io_transfer_to_stm_ptr_buffer[interface & 0x0F][1] +
        pump_io_transfer_to_stm_ptr_buffer[interface & 0x0F][2]);
    pump_io_transfer_to_stm_ptr_buffer[interface & 0x0F][4] = FRAME_END_BYTE;
    return;
}

void PUMP_IO_COM_EXE(void)
{
    //uart dma transmit
    //put all bytes here
    for(rx_msg_placement_counter = 0; rx_msg_placement_counter < PROTOCOL_FRAME_NR; rx_msg_placement_counter++)
    {
        pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE)]        = pump_io_transfer_to_stm_ptr_buffer[rx_msg_placement_counter][0];
        pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE) + 1]    = pump_io_transfer_to_stm_ptr_buffer[rx_msg_placement_counter][1];
        pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE) + 2]    = pump_io_transfer_to_stm_ptr_buffer[rx_msg_placement_counter][2];
        pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE) + 3]    = pump_io_transfer_to_stm_ptr_buffer[rx_msg_placement_counter][3];
        pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE) + 4]    = pump_io_transfer_to_stm_ptr_buffer[rx_msg_placement_counter][4];
        // printf("PUMP IO COM EXE Data 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X \r\n",
        //     (uint8_t)pump_io_transfer_to_stm_buffer[rx_msg_placement_counter * BUFFER_SIZE],
        //     (uint8_t)pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE) + 1],
        //     (uint8_t)pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE) + 2],
        //     (uint8_t)pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE) + 3],
        //     (uint8_t)pump_io_transfer_to_stm_buffer[(rx_msg_placement_counter * BUFFER_SIZE) + 4]);   
    }
    // printf("\r\n");
    uart_write_bytes(UART_NUM_2, &pump_io_transfer_to_stm_buffer[0], FRAME_BUFFER_SIZE);
    // printf("PUMP_IO_COM_EXE\r\n");
}

bool PUMP_IO_COM_check_busy_flag(void)
{
    return false;
}