#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "state_manager.h"
#include "stepper_motor.h"

// 传感器配置
#define SENSOR_L_GPIO       GPIO_NUM_2
#define SENSOR_R_GPIO       GPIO_NUM_4

// 传感器状态结构体
typedef struct {
    gpio_num_t gpio;
    const char* name;
    volatile bool last_state;
} sensor_t;

// 初始化函数
void init_proximity_switches(void);
// 中断处理任务
void sensor_trigger_task(void* pvParameters);

#endif // SENSORS_H