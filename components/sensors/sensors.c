#include <stdio.h>

#include "sensors.h"

extern int current_dir;
extern int current_spd;
extern int target_spd;
extern motor_state_t current_status;
extern SemaphoreHandle_t mutexHandle;

// 初始化传感器数组
sensor_t sensors[] = {
    {SENSOR_L_GPIO, "左限位", 1},
    {SENSOR_R_GPIO, "右限位", 1},
};
const int sensor_count = sizeof(sensors) / sizeof(sensors[0]);

// 创建事件组
EventGroupHandle_t sensor_event_group;
#define L_LIMIT_BIT (1 << 0)
#define R_LIMIT_BIT (1 << 1)


// 左右限位中断处理函数
static void IRAM_ATTR left_sensor_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xEventGroupSetBitsFromISR(sensor_event_group, L_LIMIT_BIT, &xHigherPriorityTaskWoken);
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

static void IRAM_ATTR right_sensor_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xEventGroupSetBitsFromISR(sensor_event_group, R_LIMIT_BIT, &xHigherPriorityTaskWoken);
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

// 初始化所有传感器
void init_proximity_switches() {
    sensor_event_group = xEventGroupCreate();  // 创建事件组
    gpio_install_isr_service(0);  // 安装中断服务

    for (int i = 0; i < sensor_count; i++) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << sensors[i].gpio),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_NEGEDGE  // 下降沿触发
        };
        gpio_config(&io_conf);

         // 为左右限位分别注册不同的中断处理函数
        if (sensors[i].gpio == SENSOR_L_GPIO) {
            gpio_isr_handler_add(sensors[i].gpio, left_sensor_isr_handler, (void*)sensors[i].gpio);
        } else if (sensors[i].gpio == SENSOR_R_GPIO) {
            gpio_isr_handler_add(sensors[i].gpio, right_sensor_isr_handler, (void*)sensors[i].gpio);
        }
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

void sensor_trigger_task(void* pvParameters) {
    EventBits_t event_bits;
    
    while(1) {
        // 等待任意一个限位触发事件（左或右）
        event_bits = xEventGroupWaitBits(
            sensor_event_group,
            L_LIMIT_BIT | R_LIMIT_BIT,
            pdTRUE,   // 自动清除事件位
            pdFALSE,  // 任意一个触发
            portMAX_DELAY
        );
        
        if ((event_bits & (L_LIMIT_BIT | R_LIMIT_BIT)) == (L_LIMIT_BIT | R_LIMIT_BIT)){
            if(xSemaphoreTake(mutexHandle, portMAX_DELAY) == pdTRUE){
                current_status = STATE_STOPPED;
                xSemaphoreGive(mutexHandle);
            }   
            printf("左右限位同时触发，存在干扰！\n");
        }
        else if(event_bits & L_LIMIT_BIT) {
            // printf("左限位触发！\n");
            if(xSemaphoreTake(mutexHandle, portMAX_DELAY) == pdTRUE){
                current_status = STATE_LIMIT_STOPPED;
                current_dir = DIRECTION_R;
                target_spd = 0;
                xSemaphoreGive(mutexHandle);
            }          
            current_spd = set_motor_speed(target_spd);
            set_motor_direction(current_dir);

            if(current_status == STATE_LIMIT_STOPPED) {
                if(xSemaphoreTake(mutexHandle, portMAX_DELAY) == pdTRUE){
                    target_spd = MAX_RPM;
                    current_status = STATE_ACCELERATING;
                    xSemaphoreGive(mutexHandle);
                }
            }
        }
        else if(event_bits & R_LIMIT_BIT) {            
            // printf("右限位触发！\n");
            if(xSemaphoreTake(mutexHandle, portMAX_DELAY) == pdTRUE){
                current_status = STATE_LIMIT_STOPPED;
                current_dir = DIRECTION_L;
                target_spd = 0;
                xSemaphoreGive(mutexHandle);
            }                      
            current_spd = set_motor_speed(target_spd);
            set_motor_direction(current_dir);

            if(current_status == STATE_LIMIT_STOPPED) {
                if(xSemaphoreTake(mutexHandle, portMAX_DELAY) == pdTRUE){
                    target_spd = MAX_RPM;
                    current_status = STATE_ACCELERATING;
                    xSemaphoreGive(mutexHandle);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}