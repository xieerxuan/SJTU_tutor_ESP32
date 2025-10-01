#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "sensors.h"
#include "stepper_motor.h"
#include "wifi.h"
#include "state_manager.h"

int current_dir = DIRECTION_R;
int current_spd = 0;
motor_state_t current_status = STATE_ACCELERATING;

void app_main(void)
{
    init_proximity_switches();
    init_stepper_motor();
    init_wifi();

    // 启动限位中断
    xTaskCreate(limits_isr_task, "limits_isr_task", 2048, NULL, 10, NULL);
    // 启动WiFi事件处理任务
    // xtaskCreate(wifi_event_task, "wifi_event_task", 4096, NULL, 5, NULL);

    while(1){
        state_switch_task();
    }

    
    /*  电机测试！！！！！！！！！！！！
    set_motor_direction(DIRECTION_L);
    set_motor_speed(180);
    */



    /*  传感器测试！！！！！！！！！！！
    printf("限位传感器监控已启动...\n");
    printf("----------------------------\n");
    
    while (1) {
        uint8_t any_triggered = 0;
        
        for (int i = 0; i < sensor_count; i++) {
            // 读取当前状态
            uint8_t current_state = read_sensor_state(sensors[i].gpio);
            
            // 仅当状态变化时打印
            if (current_state != sensors[i].last_state) {
                sensors[i].last_state = current_state;
                
                if (current_state == 0) {
                    printf("[触发] %s 已触发！\n", sensors[i].name);
                } else {
                    printf("[释放] %s 已释放\n", sensors[i].name);
                }
            }
            
            // 检查是否有任一传感器触发
            if (current_state == 0) {
                any_triggered = 1;
            }
        }
        
        // 显示所有传感器状态
        printf("\n当前状态: ");
        for (int i = 0; i < sensor_count; i++) {
            printf("%s:%d ", sensors[i].name, sensors[i].last_state);
        }
        printf(any_triggered ? " [警告]限位触发!" : " [正常]");
        printf("\n----------------------------\n");
        
        vTaskDelay(pdMS_TO_TICKS(200)); // 每200ms检测一次
    }     
    */

}