#include "sensors.h"
#include <stdio.h>

// 初始化传感器数组
sensor_t sensors[] = {
    {SENSOR_L_GPIO, "左限位", 1},
    {SENSOR_R_GPIO, "右限位", 1},
    // {SENSOR_M_GPIO, "零位", 1},
};

const int sensor_count = sizeof(sensors) / sizeof(sensors[0]);

// 初始化所有传感器
void init_proximity_switches() {
    for (int i = 0; i < sensor_count; i++) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << sensors[i].gpio),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_NEGEDGE  // 下降沿触发
        };
        gpio_config(&io_conf);
    }
}

bool is_left_sensor_triggered(){
    uint8_t state = gpio_get_level(SENSOR_L_GPIO);
    return !state;  // state == 0 是触发
}

bool is_right_sensor_triggered(){
    uint8_t state = gpio_get_level(SENSOR_R_GPIO);
    return !state;
}