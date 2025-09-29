// void test_back_and_forth(){
//     set_motor_speed(200);

//     while(1){
//         if(is_left_sensor_triggered()){
//             printf("左传感器被触发!!!!\n");

//             // 左传感器被触发 且滑块往左运动 --> 立即停止，调转方向
//             if(current_dir == DIRECTION_L){
//                 set_motor_speed(0);
//                 current_dir = set_motor_direction(DIRECTION_R);
//                 vTaskDelay(pdMS_TO_TICKS(1000));        // 保持停止1s
//             }
//             // 左右传感器同时被触发 --> 存在干扰，立即停止
//             if(is_right_sensor_triggered()){
//                 printf("左右传感器同时被触发!!!!\n");
//                 set_motor_speed(0);
//                 vTaskDelay(pdMS_TO_TICKS(1000));        // 保持停止1s
//             }
//             else{
//                 set_motor_speed(200);
//             }

//         }

//         else if (is_right_sensor_triggered()){
//             printf("右传感器被触发!!!!\n");

//             if(current_dir == DIRECTION_R){
//                 set_motor_speed(0);
//                 current_dir = set_motor_direction(DIRECTION_L);
//                 vTaskDelay(pdMS_TO_TICKS(1000));        // 保持停止1s
//             }
//             else{
//                 set_motor_speed(200);
//             }
            
//         }

//         else{
//             set_motor_speed(200);
//         }
//     }

// }