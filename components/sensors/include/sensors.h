#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 传感器配置
#define SENSOR_L_GPIO       GPIO_NUM_2
#define SENSOR_R_GPIO       GPIO_NUM_4
// #define SENSOR_M_GPIO       GPIO_NUM_5

// 传感器状态结构体
typedef struct {
    gpio_num_t gpio;
    const char* name;
    volatile bool last_state;
} sensor_t;

// 传感器数量
extern const int sensor_count;

// 传感器数组
extern sensor_t sensors[];

// 初始化函数
void init_proximity_switches(void);

// 读取传感器状态
uint8_t read_sensor_state(gpio_num_t pin);



#endif // SENSORS_H