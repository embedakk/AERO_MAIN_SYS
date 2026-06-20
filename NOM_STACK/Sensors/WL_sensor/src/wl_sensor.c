#include "../inc/wl_sensor.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"

static inline uint32_t get_time_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec;
}

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
uint32_t distance;
esp_err_t res;
ultrasonic_sensor_t sensor = 
{
    .trigger_pin = WL_SENSOR_TRIGGER_PIN,
    .echo_pin = WL_SENSOR_ECHO_PIN
};

void ultrasonic_init(const ultrasonic_sensor_t *dev)
{
    gpio_pad_select_gpio( dev->trigger_pin );
    gpio_pad_select_gpio( dev->echo_pin );
    gpio_set_direction(dev->trigger_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(dev->echo_pin, GPIO_MODE_INPUT);

    gpio_set_level(dev->trigger_pin, 0);
}

esp_err_t ultrasonic_measure_cm(const ultrasonic_sensor_t *dev, uint32_t max_distance, uint32_t *distance)
{
    if (!distance)
        return ESP_ERR_INVALID_ARG;

    portENTER_CRITICAL(&mux);

    // Ping: Low for 2..4 us, then high 10 us
    gpio_set_level(dev->trigger_pin, 0);
    ets_delay_us(TRIGGER_LOW_DELAY);
    gpio_set_level(dev->trigger_pin, 1);
    ets_delay_us(TRIGGER_HIGH_DELAY);
    gpio_set_level(dev->trigger_pin, 0);

    // Previous ping isn't ended
    if (gpio_get_level(dev->echo_pin))
        RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_PING);

    // Wait for echo
    uint32_t start = get_time_us();
    while (!gpio_get_level(dev->echo_pin))
    {
        if (timeout_expired(start, PING_TIMEOUT))
            RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_PING_TIMEOUT);
    }

    // got echo, measuring
    uint32_t echo_start = get_time_us();
    uint32_t time = echo_start;
    uint32_t meas_timeout = echo_start + max_distance * ROUNDTRIP;
    while (gpio_get_level(dev->echo_pin))
    {
        time = get_time_us();
        if (timeout_expired(echo_start, meas_timeout))
            RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_ECHO_TIMEOUT);
    }       
    portEXIT_CRITICAL(&mux);

    *distance = (time - echo_start) / ROUNDTRIP;

    return ESP_OK;
}

void WL_SENSOR_INIT(void)
{

    // ultrasonic_init(&sensor);
    return;
}

float WL_SENSOR_GET_DISTANCE_CM(void)
{
    // res = ultrasonic_measure_cm(&sensor, MAX_DISTANCE_CM, &distance);
    // if (res != ESP_OK)
    // {
    //     //add diagnostic here
    //     return 0.0f;
    // }else
    // {
    //     return  (float) (distance / 1.0);
    // }
    return (float)pump_io_get_distance_data;
}
