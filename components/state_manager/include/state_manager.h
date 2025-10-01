#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stepper_motor.h"

typedef enum {
    STATE_STOPPED,
    STATE_CRUISING,
    STATE_ACCELERATING,
    STATE_DECELERATING,
} motor_state_t;

void state_switch_task(void);

#endif // STATE_MANAGER_H