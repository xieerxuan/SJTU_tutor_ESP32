#include "encoder.h"
#include "esp_log.h"

static const char *TAG = "ENCODER";

// 编码器实例
static encoder_t encoder = {
    .pulse_count = 0,
    .speed_rpm = 0.0f,
    .direction = ENCODER_DIR_STOP
};

// PCNT单元配置
#define ENCODER_PCNT_UNIT PCNT_UNIT_0
#define PCNT_HIGH_LIMIT 10000
#define PCNT_LOW_LIMIT  -10000

// 编码器初始化
void encoder_init(void) {
    // 配置A相脉冲计数器
    pcnt_config_t pcnt_config_a = {
        .pulse_gpio_num = ENCODER_A_GPIO,
        .ctrl_gpio_num = ENCODER_B_GPIO,  // B相作为控制信号
        .lctrl_mode = PCNT_MODE_REVERSE,  // 当B相为低电平时，递减计数
        .hctrl_mode = PCNT_MODE_KEEP,     // 当B相为高电平时，递增计数
        .pos_mode = PCNT_COUNT_INC,       // 上升沿递增
        .neg_mode = PCNT_COUNT_DIS,       // 下降沿不计数
        .counter_h_lim = PCNT_HIGH_LIMIT,
        .counter_l_lim = PCNT_LOW_LIMIT,
        .unit = ENCODER_PCNT_UNIT,
        .channel = PCNT_CHANNEL_0,
    };
    pcnt_unit_config(&pcnt_config_a);
    
    // 配置滤波器以减少抖动
    pcnt_set_filter_value(ENCODER_PCNT_UNIT, 100);
    pcnt_filter_enable(ENCODER_PCNT_UNIT);
    
    // 启动PCNT单元
    pcnt_counter_pause(ENCODER_PCNT_UNIT);
    pcnt_counter_clear(ENCODER_PCNT_UNIT);
    pcnt_counter_resume(ENCODER_PCNT_UNIT);
    
    ESP_LOGI(TAG, "Encoder initialized successfully");
}

// 获取当前速度(RPM)
float encoder_get_speed(void) {
    static int16_t prev_count = 0;
    static uint32_t prev_time = 0;
    
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    int16_t current_count = 0;
    
    // 读取当前脉冲计数
    pcnt_get_counter_value(ENCODER_PCNT_UNIT, &current_count);
    
    // 计算时间差(转换为秒)
    float time_diff = (current_time - prev_time) / 1000.0f;
    
    if (time_diff > (ENCODER_SAMPLE_MS / 1000.0f)) {
        if (time_diff > 0) {
            // 计算脉冲差
            int16_t pulse_diff = current_count - prev_count;
            
            // 计算转速(RPM): (脉冲差/每转脉冲数) / 时间差(分钟)
            encoder.speed_rpm = (pulse_diff / (float)ENCODER_PPR) / (time_diff / 60.0f);
            
            // 确定方向
            if (pulse_diff > 0) {
                encoder.direction = ENCODER_DIR_CW;
            } else if (pulse_diff < 0) {
                encoder.direction = ENCODER_DIR_CCW;
            } else {
                encoder.direction = ENCODER_DIR_STOP;
            }
        }
        
        prev_count = current_count;
        prev_time = current_time;
    }
    
    return encoder.speed_rpm;
}

encoder_direction_t encoder_get_direction(void) {
    encoder_get_speed();
    return encoder.direction;
}

int32_t encoder_get_pulse_count(void) {
    int16_t count = 0;
    pcnt_get_counter_value(ENCODER_PCNT_UNIT, &count);
    return count;
}

void encoder_reset_pulse_count(void) {
    pcnt_counter_clear(ENCODER_PCNT_UNIT);
}

void test_encoder() {
    float speed = encoder_get_speed();        
    encoder_direction_t direction = encoder_get_direction();
    int32_t pulse_count = encoder_get_pulse_count();
    
    const char* dir_str;
    switch (direction) {
        case ENCODER_DIR_CW:
            dir_str = "顺时针";
            break;
        case ENCODER_DIR_CCW:
            dir_str = "逆时针";
            break;
        case ENCODER_DIR_STOP:
            dir_str = "停止";
            break;
        default:
            dir_str = "未知";
    }
    
    printf("速度: %7.2f RPM, 方向: %s, 脉冲计数: %6ld\n", 
            speed, dir_str, pulse_count);
    
    vTaskDelay(pdMS_TO_TICKS(500));
}
