#include "stepper_motor.h"

static const char *TAG = "StepperMotor";

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
        .duty = 0, // 初始占空比为0
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);
    
    // 初始化电机状态：禁用电机，设置方向，停止PWM
    gpio_set_level(ENABLE_PIN, 1); // 禁用电机（根据驱动器逻辑，可能为高或低电平使能）
    gpio_set_level(DIR_PIN, 0);    // 初始方向
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void set_motor_direction(int direction){
    if (direction == 1) {
        gpio_set_level(DIR_PIN, 0); // 正转
    } else if (direction == -1) {
        gpio_set_level(DIR_PIN, 1); // 反转
    }
}

void set_motor_speed(int rpm)
{
    // 限制转速范围
    if (rpm > MAX_RPM) rpm = MAX_RPM;
    if (rpm < 0) rpm = -rpm;
    
    // 计算所需的PWM频率
    uint32_t frequency = (rpm * STEPS_PER_REV) / 60;

    if (frequency == 0) {
        // 停止电机
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        gpio_set_level(ENABLE_PIN, 1); // 禁用电机
    } else {
        // 启用电机
        gpio_set_level(ENABLE_PIN, 0); // 使能电机
        
        // 更新PWM频率和占空比
        ledc_set_freq(LEDC_MODE, LEDC_TIMER, frequency);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    }    
}