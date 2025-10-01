#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include "driver/ledc.h"
#include "driver/gpio.h"

// 电机参数定义
#define STEPS_PER_REV        200     // 200步/圈 (1.8°步距角)
#define MAX_RPM              600     // 最高转速600RPM
#define MIN_RPM              60

#define MICRO_STEPS         8
#define DIRECTION_R         0
#define DIRECTION_L         1

// GPIO引脚定义（请根据实际硬件连接修改）
#define PWM_PIN             GPIO_NUM_19    // 脉冲信号引脚
#define DIR_PIN             GPIO_NUM_18    // 方向信号引脚
#define ENABLE_PIN          GPIO_NUM_5     // 使能信号引脚

// LEDC PWM配置
#define LEDC_CHANNEL        LEDC_CHANNEL_0
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES       LEDC_TIMER_4_BIT // 4位分辨率
#define LEDC_FREQUENCY      4000              // 初始频率4000Hz
#define LEDC_DUTY           8                // 50%占空比 (2^4 = 16, 50% = 8)

void init_stepper_motor(void);

void set_motor_direction(int direction);

void set_motor_speed(int rpm);  // rpm>0

#endif