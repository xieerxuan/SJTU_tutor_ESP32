#include <stdio.h>
#include "state_manager.h"

extern int current_dir;
extern int current_spd;
extern int target_spd;
extern motor_state_t current_status;
extern SemaphoreHandle_t mutexHandle;

void state_switch_task(void *pvParameters)
{
    while(1){
        switch(current_status){
            case STATE_LIMIT_STOPPED:
                break;
            case STATE_STOPPED:
                if(xSemaphoreTake(mutexHandle, portMAX_DELAY) == pdTRUE){ 
                    target_spd = 0;
                    current_spd = set_motor_speed(target_spd);
                    xSemaphoreGive(mutexHandle);
                }
                break;
            case STATE_CRUISING:
                // printf("匀速中，当前转速：%d RPM\n", current_spd);
                if(xSemaphoreTake(mutexHandle, portMAX_DELAY) == pdTRUE){ 
                    target_spd = current_spd;
                    current_spd = set_motor_speed(target_spd);
                    xSemaphoreGive(mutexHandle);
                }
                break;
            case STATE_ACCELERATING:
                // printf("加速中，当前转速：%d RPM\n", current_spd);
                /* TODO */
                /* 在此处实现电机加速功能 */
            default:
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
