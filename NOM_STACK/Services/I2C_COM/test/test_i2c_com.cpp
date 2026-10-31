#include <gtest/gtest.h>
#include <cstring>

#include "mock_esp_state.h"
#include "../inc/i2c_com.h"

class I2cComTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
    }
};

TEST_F(I2cComTest, ReadAndWriteAlwaysFreeCmdLinkOnSuccessAndFailure) {
    i2c_dev_t dev = {};
    dev.port = I2C_NUM_1;
    dev.addr = 0x68;

    uint8_t tx_val = 0x42;
    EXPECT_EQ(i2c_dev_write_reg(&dev, 0x05, &tx_val, 1), ESP_OK);
    EXPECT_EQ(mock_i2c_get_cmd_alloc_count(), 1U);
    EXPECT_EQ(mock_i2c_get_cmd_free_count(), 1U);

    uint8_t rx_val = 0x00;
    EXPECT_EQ(i2c_dev_read_reg(&dev, 0x05, &rx_val, 1), ESP_OK);
    EXPECT_EQ(rx_val, 0x42);
    EXPECT_EQ(mock_i2c_get_cmd_alloc_count(), 2U);
    EXPECT_EQ(mock_i2c_get_cmd_free_count(), 2U);

    // Inject I2C bus failure and verify cmd link is still freed (no heap leak)
    mock_i2c_set_next_result(ESP_FAIL);
    EXPECT_EQ(i2c_dev_read_reg(&dev, 0x05, &rx_val, 1), ESP_FAIL);
    EXPECT_EQ(i2c_dev_write_reg(&dev, 0x05, &tx_val, 1), ESP_FAIL);
    EXPECT_EQ(mock_i2c_get_cmd_alloc_count(), 4U);
    EXPECT_EQ(mock_i2c_get_cmd_free_count(), 4U);
}

TEST_F(I2cComTest, NullAndZeroLengthArgumentsAreRejectedWithoutLeak) {
    i2c_dev_t dev = {I2C_NUM_1, 0x68, 21, 22, I2C_FREQ_HZ};
    uint8_t buf[2] = {0};

    EXPECT_EQ(i2c_dev_read(nullptr, nullptr, 0, buf, 2), ESP_ERR_INVALID_ARG);
    EXPECT_EQ(i2c_dev_read(&dev, nullptr, 0, nullptr, 2), ESP_ERR_INVALID_ARG);
    EXPECT_EQ(i2c_dev_read(&dev, nullptr, 0, buf, 0), ESP_ERR_INVALID_ARG);
    EXPECT_EQ(i2c_dev_write(nullptr, nullptr, 0, buf, 2), ESP_ERR_INVALID_ARG);
    EXPECT_EQ(i2c_dev_write(&dev, nullptr, 0, nullptr, 2), ESP_ERR_INVALID_ARG);
    EXPECT_EQ(i2c_dev_write(&dev, nullptr, 0, buf, 0), ESP_ERR_INVALID_ARG);
    EXPECT_EQ(mock_i2c_get_cmd_alloc_count(), 0U);
}

TEST_F(I2cComTest, SensorInputBufferBoundsGuardPreventsOverflow) {
    uint8_t valid_frame[6] = {I2C_COM_PH_SLAVE_ADDR, 0x12, 0x34, I2C_COM_EC_SLAVE_ADDR, 0xAB, 0xCD};
    memcpy(i2c_com_sensor_data_input, valid_frame, sizeof(valid_frame));
    i2c_com_sensor_data_input_len = 6;
    I2C_COM_GET_SENSOR_DATA();
    EXPECT_EQ(i2c_com_ph_data, 0x1234U);
    EXPECT_EQ(i2c_com_ec_data, 0xABCDU);

    // Oversized length (>= 10) must not index out-of-bounds
    i2c_com_ph_data = 0;
    i2c_com_sensor_data_input_len = 128;
    I2C_COM_GET_SENSOR_DATA();
    EXPECT_EQ(i2c_com_ph_data, 0U);
}
