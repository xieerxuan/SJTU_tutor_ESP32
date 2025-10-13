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
int target_spd = 0;
motor_state_t current_status = STATE_STOPPED;


void app_main(void)
{
    init_stepper_motor();
    xTaskCreate(state_switch_task, "state_switch_task", 4096, NULL, 5, NULL);   // 最高优先级任务

    init_proximity_switches();      // 注册了限位中断
    init_wifi();        // 启动了TCP发送数据（优先级1）和接收指令（优先级2）任务
    xTaskCreate(sensor_trigger_task, "sensor_trigger_task", 4096, NULL, 4, NULL);   // 次高优先级任务

    while(1){
        // Idle
    }   
}