#include "../inc/i2c_com.h"

void I2C_COM_INIT(void)
{
    
    i2c_config_t conf = 
    {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = I2C_COM_I2C_SLAVE_SDA,
        .scl_io_num = I2C_COM_I2C_SLAVE_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .slave = 
        {
            .slave_addr = I2C_COM_I2C_SLAVE_ADDR,
            .maximum_speed = 100000, // 100 kHz
        },
    };
    internal_i2c_driver_status = i2c_param_config(I2C_COM_I2C_SLAVE_NUM, &conf); 
    if(internal_i2c_driver_status != ESP_OK)
    {
        //add diagnostic error
#ifdef I2C_COM_LOGS
        ESP_LOGE(I2C_COM_TAG, "i2c_param_config ==>> ESP FAILURE !!!");
#endif
    }else
    {
#ifdef I2C_COM_LOGS
        ESP_LOGI(I2C_COM_TAG, "i2c_param_config ==>> ESP OKK ...");
#endif
    }
    internal_i2c_driver_status = i2c_driver_install(I2C_COM_I2C_SLAVE_NUM, I2C_MODE_SLAVE, I2C_COM_I2C_BUF_LEN, I2C_COM_I2C_BUF_LEN, 0);
    if(internal_i2c_driver_status != ESP_OK)
    {
        //add diagnostic error
#ifdef I2C_COM_LOGS
        ESP_LOGE(I2C_COM_TAG, "i2c_driver_install ==>> ESP FAILURE !!!");
#endif
    }else
    {
#ifdef I2C_COM_LOGS
        ESP_LOGI(I2C_COM_TAG, "i2c_driver_install ==>> ESP OKK ...");
#endif
    }
    return;
}

esp_err_t i2c_dev_init(i2c_port_t port, int sda, int scl)
{
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = sda,
		.scl_io_num = scl,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		//.master.clk_speed = 1000000
		.master.clk_speed = I2C_FREQ_HZ
	};
	//i2c_param_config(I2C_NUM_0, &i2c_config);
	//i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
	internal_i2c_driver_status = i2c_param_config(port, &i2c_config);
    if(internal_i2c_driver_status != ESP_OK)
    {
        //add diagnostic error
#ifdef I2C_COM_LOGS
        ESP_LOGE(I2C_COM_TAG, "I2C_NUM_1 i2c_param_config ==>> ESP FAILURE !!!");
#endif
    }else
    {
#ifdef I2C_COM_LOGS
        ESP_LOGI(I2C_COM_TAG, "I2C_NUM_1 i2c_param_config ==>> ESP OKK ...");
#endif
    }
    internal_i2c_driver_status = i2c_driver_install(port, I2C_MODE_MASTER, 0, 0, 0);
    if(internal_i2c_driver_status != ESP_OK)
    {
        //add diagnostic error
#ifdef I2C_COM_LOGS
        ESP_LOGE(I2C_COM_TAG, "I2C_NUM_1 i2c_driver_install ==>> ESP FAILURE !!!");
#endif
    }else
    {
#ifdef I2C_COM_LOGS
        ESP_LOGI(I2C_COM_TAG, "I2C_NUM_1 i2c_driver_install ==>> ESP OKK ...");
#endif
    }
	return internal_i2c_driver_status; //i2c_driver_install(port, I2C_MODE_MASTER, 0, 0, 0);
}

esp_err_t i2c_dev_read(const i2c_dev_t *dev, const void *out_data, size_t out_size, void *in_data, size_t in_size)
{
	if (!dev || !in_data || !in_size) return ESP_ERR_INVALID_ARG;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	if (out_data && out_size)
	{
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write(cmd, (void *)out_data, out_size, true);
	}
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev->addr << 1) | 1, true);
	i2c_master_read(cmd, in_data, in_size, I2C_MASTER_LAST_NACK);
	i2c_master_stop(cmd);

	esp_err_t res = i2c_master_cmd_begin(dev->port, cmd, I2CDEV_TIMEOUT / portTICK_PERIOD_MS);
	if (res != ESP_OK)
#ifdef I2C_COM_LOGS
		ESP_LOGE(I2C_COM_TAG, "I2C_NUM_1 Could not read from device [0x%02x at %d]: %d", dev->addr, dev->port, res);
#endif
        i2c_cmd_link_delete(cmd);

	return res;
}

esp_err_t i2c_dev_write(const i2c_dev_t *dev, const void *out_reg, size_t out_reg_size, const void *out_data, size_t out_size)
{
	if (!dev || !out_data || !out_size) return ESP_ERR_INVALID_ARG;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
	if (out_reg && out_reg_size)
		i2c_master_write(cmd, (void *)out_reg, out_reg_size, true);
	i2c_master_write(cmd, (void *)out_data, out_size, true);
	i2c_master_stop(cmd);
	esp_err_t res = i2c_master_cmd_begin(dev->port, cmd, I2CDEV_TIMEOUT / portTICK_PERIOD_MS);
	if (res != ESP_OK)
#ifdef I2C_COM_LOGS
		ESP_LOGE(I2C_COM_TAG, "I2C_NUM_1 Could not write to device [0x%02x at %d]: %d", dev->addr, dev->port, res);
#endif
        i2c_cmd_link_delete(cmd);

	return res;
}

uint8_t *I2C_COM_GET_SENSOR_DATA(void)
{
    // i2c_com_sensor_data_input_len = i2c_slave_read_buffer(I2C_COM_I2C_SLAVE_NUM, i2c_com_sensor_data_input, I2C_COM_I2C_BUF_LEN, pdMS_TO_TICKS(10));
    if (i2c_com_sensor_data_input_len > 0) 
    {
        i2c_com_sensor_data_input[i2c_com_sensor_data_input_len] = '\0';  // Null-terminate received string
#ifdef I2C_COM_LOGS
        ESP_LOGI(I2C_COM_TAG, "Sensor Input Data ==>> 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
                i2c_com_sensor_data_input[0], i2c_com_sensor_data_input[1], i2c_com_sensor_data_input[2],
                i2c_com_sensor_data_input[3], i2c_com_sensor_data_input[4], i2c_com_sensor_data_input[5]);
#endif
        if(i2c_com_sensor_data_input_len != 6)
        {
            //add diagnsotic failure
#ifdef I2C_COM_LOGS
            ESP_LOGE(I2C_COM_TAG, "I2C_COM SENSOR DATA LENGTH ==>> FAILURE !!!");
#endif
            return &i2c_com_sensor_data_input[0];
        }
        
        if(i2c_com_sensor_data_input[0] == I2C_COM_PH_SLAVE_ADDR)
        {
            i2c_com_ph_data = (uint16_t)((i2c_com_sensor_data_input[1] << 8) | i2c_com_sensor_data_input[2]);
        }else
        {
            //add diagnsotic here
#ifdef I2C_COM_LOGS
            ESP_LOGE(I2C_COM_TAG, "I2C_COM_PH_SLAVE_ADDR ==>> FAILURE !!!");
#endif
        }
        if(i2c_com_sensor_data_input[3] == I2C_COM_EC_SLAVE_ADDR)
        {
            i2c_com_ec_data = (uint16_t)((i2c_com_sensor_data_input[4] << 8) | i2c_com_sensor_data_input[5]);
        }else
        {
            //add diagnsotic here
#ifdef I2C_COM_LOGS
            ESP_LOGE(I2C_COM_TAG, "I2C_COM_EC_SLAVE_ADDR ==>> FAILURE !!!");
#endif
        }
    }else
    {
        //add diagnsotic here
#ifdef I2C_COM_LOGS
        ESP_LOGE(I2C_COM_TAG, "Sensor Input Data ==>> ESP FAILURE !!!");
#endif
    }

    return &i2c_com_sensor_data_input[0];
}