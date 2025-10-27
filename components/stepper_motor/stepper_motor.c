#include "stepper_motor.h"

extern int current_dir;
extern int current_spd;

void init_stepper_motor(void)
{    
    // 配置GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PWM_PIN) | (1ULL << DIR_PIN) | (1ULL << ENABLE_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    // 配置LEDC定时器
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);
    
    // 配置LEDC通道
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = PWM_PIN,
        .duty = LEDC_DUTY,
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);
    
    // 初始化电机状态：禁用电机，设置方向，停止PWM
    gpio_set_level(ENABLE_PIN, 0);          // 禁用电机
    gpio_set_level(DIR_PIN, DIRECTION_R);    // 初始方向
}

void set_motor_direction(int direction){
    gpio_set_level(DIR_PIN, direction);
}

int set_motor_speed(int rpm)
{
    /* TODO */
    /* 
    Tips：
        1. 在此处实现电机速度的设置，返回传入的转速，此函数规范的调用方式应为 “current_spd = set_motor_speed(your_target_speed)” ；
        2. 通过计算得到对应的PWM频率，计算公式为 f = (rpm × 每转步数) × 细分步数 / 60（相关参数宏定义在stepper_motor.h中，注意与硬件保持一致）；
        3. 注意，当转速设置为0时，应禁用电机以防止短路发热；
        4. ledc_set_freq() 函数可以用来设置PWM频率，从而控制电机速度，详见官方文档。
    */

    return rpm;
}
