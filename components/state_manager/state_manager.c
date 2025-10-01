#include <stdio.h>
#include "state_manager.h"

extern int current_dir;
extern int current_spd;
extern motor_state_t current_status;

void state_switch_task(void)
{
    switch(current_status){
        case STATE_STOPPED:
            current_spd = 0;
            // printf("电机停止，当前转速：%d RPM\n", current_spd);
            set_motor_speed(current_spd);
            vTaskDelay(pdMS_TO_TICKS(50));
            break;
        case STATE_CRUISING:
            // printf("匀速中，当前转速：%d RPM\n", current_spd);
            set_motor_speed(current_spd);
            vTaskDelay(pdMS_TO_TICKS(50));
            break;
        case STATE_ACCELERATING:
            // printf("加速中，当前转速：%d RPM\n", current_spd);
            if(current_spd < MIN_RPM){  // 启动最低转速
                current_spd = MIN_RPM;
                set_motor_speed(current_spd);
                vTaskDelay(pdMS_TO_TICKS(50));
                break;
            }
            if(current_spd < MAX_RPM){  // 加速到最高转速
                current_spd += 5;
                set_motor_speed(current_spd);
                vTaskDelay(pdMS_TO_TICKS(50));
                break;
            }
            else{
                current_spd = MAX_RPM;
                set_motor_speed(current_spd);
                current_status = STATE_CRUISING;
                vTaskDelay(pdMS_TO_TICKS(50));
                break;
            }
        case STATE_DECELERATING:
            // printf("减速中，当前转速：%d RPM\n", current_spd);
            if(current_spd > MIN_RPM){
                current_spd -= 5;
                set_motor_speed(current_spd);
                vTaskDelay(pdMS_TO_TICKS(50));
                break;
            }
            else{
                current_spd = MIN_RPM;
                set_motor_speed(current_spd);
                current_status = STATE_CRUISING;
                vTaskDelay(pdMS_TO_TICKS(50));
                break;
            }
    }
}
