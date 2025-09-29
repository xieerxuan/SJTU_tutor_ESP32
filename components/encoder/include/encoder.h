#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/pcnt.h"

#include "stepper_motor.h"

// 编码器配置
#define ENCODER_A_GPIO       GPIO_NUM_12
#define ENCODER_B_GPIO       GPIO_NUM_13
#define ENCODER_PPR          600         // 每转脉冲数
#define ENCODER_SAMPLE_MS    100         // 速度采样周期(毫秒)

// 编码器方向定义
typedef enum {
    ENCODER_DIR_CW = 1,     // 顺时针
    ENCODER_DIR_CCW = -1,   // 逆时针
    ENCODER_DIR_STOP = 0    // 停止
} encoder_direction_t;

// 编码器结构体
typedef struct {
    int32_t pulse_count;        // 脉冲计数
    float speed_rpm;           // 转速(RPM)
    encoder_direction_t direction;  // 转动方向
} encoder_t;


void encoder_init(void);

float encoder_get_speed(void);

encoder_direction_t encoder_get_direction(void);

// 获取脉冲计数(可用于位置跟踪)
int32_t encoder_get_pulse_count(void);

// 重置脉冲计数
void encoder_reset_pulse_count(void);

void test_encoder(void);

#endif // ENCODER_H