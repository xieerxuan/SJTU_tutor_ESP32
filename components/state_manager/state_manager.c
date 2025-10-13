#include <stdio.h>
#include "state_manager.h"

extern int current_dir;
extern int current_spd;
extern int target_spd;
extern motor_state_t current_status;

void state_switch_task(void *pvParameters)
{
    switch(current_status){
        case STATE_LIMIT_STOPPED:
            break;
        case STATE_STOPPED:
            target_spd = 0;
            current_spd = set_motor_speed(target_spd);
            vTaskDelay(pdMS_TO_TICKS(50));
            break;
        case STATE_CRUISING:
            // printf("匀速中，当前转速：%d RPM\n", current_spd);
            target_spd = current_spd;
            current_spd = set_motor_speed(target_spd);
            vTaskDelay(pdMS_TO_TICKS(50));
            break;
        case STATE_ACCELERATING:
            // printf("加速中，当前转速：%d RPM\n", current_spd);
            if(target_spd > MAX_RPM || target_spd < MIN_RPM){
                printf("目标转速 %d RPM 超出范围\n", target_spd);
                break;
            }

            int diff_spd = target_spd - current_spd;
            if(diff_spd < 5 && diff_spd > -5){
                current_spd = set_motor_speed(target_spd);
                current_status = STATE_CRUISING;
                vTaskDelay(pdMS_TO_TICKS(50));
                break;
            }
            else if(diff_spd > 0){
                current_spd += 5;
                set_motor_speed(current_spd);
                vTaskDelay(pdMS_TO_TICKS(50));
                break;
            }
            else{
                current_spd -= 5;
                set_motor_speed(current_spd);
                vTaskDelay(pdMS_TO_TICKS(50));
                break;
            }
    }
}
