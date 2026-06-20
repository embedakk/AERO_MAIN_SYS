// // #include <stdio.h>

// // void app_main(void) {
// // 	printf("Hello world!\n");
// // }
// #include <stdio.h>
// #include <stddef.h>
// #include <stdint.h>
// #include <stdbool.h>
// #include <string.h>

// #include "esp_log.h"
// #include "esp_system.h"

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"
// #include "freertos/semphr.h"

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/i2c.h"
// #include "esp_log.h"

// #include "driver/i2c.h"

// #define	PH_EC_TEST	"TEST_SENSOR_INT"

// #define PH_SLAVE_ADDR	0x22//0xAA // 170
// #define EC_SLAVE_ADDR	0x24    //0xAB // 171
// #define ESP_SLAVE_TIMEOUT 5000 / portTICK_PERIOD_MS  // Timeout



// #define I2C_SLAVE_NUM    I2C_NUM_0   // I2C Port Number
// #define I2C_SLAVE_SDA    21          // SDA Pin
// #define I2C_SLAVE_SCL    22          // SCL Pin
// #define I2C_SLAVE_ADDR   0x08        // I2C Slave Address
// #define I2C_BUF_LEN      128         // Buffer Size

// static const char *TAG = "I2C_Slave";

// uint8_t rx_buffer[I2C_BUF_LEN] = {0};
// uint8_t tx_buffer[I2C_BUF_LEN] = "ESP32 Ready!"; // Default response
// uint8_t tx_w_buffer[I2C_BUF_LEN] = "Hello Stm32!"; // Default response

// // I2C Slave Initialization
// void i2c_slave_init() {
//     i2c_config_t conf = {
//         .mode = I2C_MODE_SLAVE,
//         .sda_io_num = I2C_SLAVE_SDA,
//         .scl_io_num = I2C_SLAVE_SCL,
//         .sda_pullup_en = GPIO_PULLUP_ENABLE,
//         .scl_pullup_en = GPIO_PULLUP_ENABLE,
//         .slave = {
//             .slave_addr = I2C_SLAVE_ADDR,
//             .maximum_speed = 100000, // 100 kHz
//         },
//     };

//     ESP_ERROR_CHECK(i2c_param_config(I2C_SLAVE_NUM, &conf));
//     ESP_ERROR_CHECK(i2c_driver_install(I2C_SLAVE_NUM, I2C_MODE_SLAVE, I2C_BUF_LEN, I2C_BUF_LEN, 0));
// }
// bool data_sent = false;
// // Task to Handle I2C Read/Write Operations
// void i2c_slave_task(void *arg) {
//     while (1) {
//         // Check for data received from Master
//         int len = i2c_slave_read_buffer(I2C_SLAVE_NUM, rx_buffer, I2C_BUF_LEN, pdMS_TO_TICKS(5));
//         if (len > 0) {
//             rx_buffer[len] = '\0';  // Null-terminate received string
//             ESP_LOGI(TAG, "Received from Master: %s", rx_buffer);

//             // Prepare a response for the next read request
//             // snprintf((char *)tx_w_buffer, 140, "ACK: %s", rx_buffer);
//             // data_sent = false;  // Reset flag when new data comes in
//         }

//         // if(data_sent == false)
//         // {
//             // If Master Requests Data (reads from ESP32)
//             int len_w = i2c_slave_write_buffer(I2C_SLAVE_NUM, tx_w_buffer, strlen((char *)tx_w_buffer), pdMS_TO_TICKS(5)); 

//             if ( len_w > 0) {
//                 // ESP_LOGI(TAG, "len_w = %d Master Requested Data: Sending -> %s", len_w, tx_w_buffer);
//                 // data_sent = true;  // Mark as sent
//             }
//         // }


//         vTaskDelay(pdMS_TO_TICKS(10)); // Prevent CPU overload
//     }
// }

// void cpu_core1_task(void *arg) 
// {
//     while(1)
//     {
//         ESP_LOGI(TAG, "cpu_core1_task...");
//         vTaskDelay(10 / portTICK_PERIOD_MS);
//     }
// }

// void cpu_core2_task(void *arg) 
// {
//     while(1)
//     {
//         ESP_LOGI(TAG, "cpu_core2_task...");
//         vTaskDelay(10 / portTICK_PERIOD_MS);
//     }
// }

// void app_main(void)
// {
//     // int i = 0;
// 	// resp_i2c = i2c_master_init();
//     // if(resp_i2c == ESP_OK)
//     // {
//     //     ESP_LOGW(PH_EC_TEST, "i2c_master_init SUCCESS...\r\n");
//     // }
//     // else
//     // {
//     //     ESP_LOGW(PH_EC_TEST, "i2c_master_init ERROR!!!\r\n");
//     // }
//     // while (1) {
//     //     printf("[%d] PH_SLAVE_ADDR READ!\n", i);
// 	// 	resp_i2c = i2c_master_read_from_device(0, PH_SLAVE_ADDR, (uint8_t *)&i2c_data[0], 2, 1200/portTICK_RATE_MS); 
// 	// 	 if(resp_i2c == ESP_OK)
//     //  	{
//     //         ESP_LOGW(PH_EC_TEST, "read from PH device SUCCESS...\r\n");
//     //     }
//     //     else
//     //     {
//     //         ESP_LOGW(PH_EC_TEST, "read from PH device ERROR!!!\r\n");
//     //     }
// 	// 	vTaskDelay(2000 / portTICK_PERIOD_MS);

// 	// 	printf("[%d] EC_SLAVE_ADDR READ!\n", i);
// 	// 	resp_i2c = i2c_master_read_from_device(0, EC_SLAVE_ADDR, (uint8_t *)&i2c_data[0], 2, 1200/portTICK_RATE_MS); 
// 	// 	 if(resp_i2c == ESP_OK)
//     //  	{
//     //         ESP_LOGW(PH_EC_TEST, "read from EC DEVICE SUCCESS...\r\n");
//     //     }
//     //     else
//     //     {
//     //         ESP_LOGW(PH_EC_TEST, "read from EC DEVICE ERROR!!!\r\n");
//     //     }
// 	// 	vTaskDelay(2000 / portTICK_PERIOD_MS);
//     //     i++;
        
//     // }

//     // i2c_master_init();
//     // xTaskCreate(i2c_task, "i2c_task", 4096, NULL, 5, NULL);

//     // uint8_t data[I2C_BUF_LEN];

//     // // Initialize I2C Slave
//     // i2c_slave_init();
//     // ESP_LOGI(TAG, "I2C Slave Initialized");

//     // while (1) {
//     //     int len = i2c_slave_read_buffer(I2C_SLAVE_NUM, data, I2C_BUF_LEN, pdMS_TO_TICKS(500));

//     //     if (len > 0) {
//     //         data[len] = '\0'; // Null terminate for string
//     //         ESP_LOGI(TAG, "Received: %s", data);
//     //     }

//     //     vTaskDelay(pdMS_TO_TICKS(100));
//     // }
//     i2c_slave_init();
//     ESP_LOGI(TAG, "I2C Slave Initialized");

//     xTaskCreate(i2c_slave_task, "i2c_slave_task", 4096, NULL, 5, NULL);
//     xTaskCreatePinnedToCore(cpu_core1_task, "cpu_core1_task", 4096, NULL, 5, NULL, 0);
//     xTaskCreatePinnedToCore(cpu_core2_task, "cpu_core2_task", 4096, NULL, 5, NULL, 1);
//     // xTaskCreatePinnedToCore()
// }


///////

#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "../NOM_STACK/Manager/INIT/inc/initialization.h"
#include "../NOM_STACK/Services/Scheduler/inc/scheduler.h"


#define GPIO_OUTPUTA_PIN 21  // Change this to your desired GPIO pin
#define GPIO_OUTPUTB_PIN 22

static const char *TAG = "TASK_TEST";

// bool led_state_a = false, led_state_b = false;
bool ON_SECOND_TASK = false;


uint8_t uart_write_data[8] = {0x5A, 0xA5, 0x05, 0x82, 0x50, 0x70, 0x00, 0x01};
uint8_t uart_write_data2[10] = {0x5A, 0xA5, 0x07, 0x82, 0x50, 0x20, 0x41, 0xCE, 0x00, 0x00};

void app_main() 
{
    // gpio_reset_pin(GPIO_OUTPUTA_PIN);
    // gpio_set_direction(GPIO_OUTPUTA_PIN, GPIO_MODE_OUTPUT);
    // gpio_reset_pin(GPIO_OUTPUTB_PIN);
    // gpio_set_direction(GPIO_OUTPUTB_PIN, GPIO_MODE_OUTPUT);

    // xTaskCreate(task_scheduler, "Scheduler", 4096, NULL, 10, NULL);
    // INIT_PERIPHERALS();
    SCHEDULER_INIT();
    SCHEDULER_MAIN();
}




// #include "freertos/FreeRTOS.h"
// #include "freertos/queue.h"
// #include "driver/uart.h"
// #include "esp_log.h"

// #define UART_NUM          UART_NUM_1
// #define BUF_SIZE          1024
// // GPIO 9 (TX) & GPIO 10 (RX)
// #define TX_PIN            27
// #define RX_PIN            33

// static const char *TAG = "UART_DMA";
// static QueueHandle_t uart_queue;

// // ISR Handler (Handles UART Events)
// static void IRAM_ATTR uart_isr_handler(void *arg) {
//     uart_event_t event;
//     BaseType_t xHigherPriorityTaskWoken = pdFALSE;

//     if (xQueueReceiveFromISR(uart_queue, &event, &xHigherPriorityTaskWoken)) {
//         if (event.type == UART_DATA) {
//             // ESP_LOGI(TAG, "ISR: New UART Data, Length: %d", event.size);
//         }
//     }

//     portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
// }

// static void rx_task(void *arg)
// {
//     while (1)
//     {
//         uart_event_t event;
//         if (xQueueReceive(uart_queue, &event, portMAX_DELAY)) 
//         {
//             if (event.type == UART_DATA) 
//             {
//                 uint8_t *dma_buffer;
//                 size_t len;

//                 // ✅ Zero-Copy: Get DMA buffer pointer (No CPU copying!)
//                 uart_get_buffered_data_len(UART_NUM, &len);
//                 dma_buffer = (uint8_t *)malloc(len);

//                 if (dma_buffer) 
//                 {
//                     uart_read_bytes(UART_NUM, dma_buffer, len, 0);
//                     // ESP_LOGI(TAG, "Received (DMA Buffer):"); // %.*s", len, dma_buffer);
//                     for(uint8_t counter = 0; counter < len; counter++)
//                     {
//                         printf("0x%02X, ", dma_buffer[counter]);
//                     }
//                     printf("\r\n");
//                     free(dma_buffer);  // Free buffer after processing
//                 }
//             }
//         }
//     }
// }

// uint8_t uart_write_data[8] = {0x5A, 0xA5, 0x05, 0x82, 0x50, 0x70, 0x00, 0x01};
// uint8_t uart_write_data2[10] = {0x5A, 0xA5, 0x07, 0x82, 0x50, 0x20, 0x41, 0xCE, 0x00, 0x00};
// void app_main(void) {
//     // UART Configuration
//     uart_config_t uart_config = {
//         .baud_rate = 115200,
//         .data_bits = UART_DATA_8_BITS,
//         .parity = UART_PARITY_DISABLE,
//         .stop_bits = UART_STOP_BITS_1,
//         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
//     };

//     uart_param_config(UART_NUM, &uart_config);
//     uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

//     // Enable UART Driver with DMA (Double Buffer)
//     uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart_queue, 0);
//     uart_enable_rx_intr(UART_NUM);

//     // ESP_LOGI(TAG, "UART DMA RX Initialized!");
//     xTaskCreate(rx_task, "ui_protocol_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);

//     while (1) 
//     {
//         uart_write_bytes(UART_NUM, uart_write_data, 8);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//         uart_write_bytes(UART_NUM, uart_write_data2, 10);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }

