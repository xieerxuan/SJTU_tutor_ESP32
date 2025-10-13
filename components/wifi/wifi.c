#include <stdio.h>
#include "wifi.h"

extern int current_dir;
extern int current_spd;
extern int target_spd;
extern motor_state_t current_status;

static int sock_listen = -1;
static int sock_client = -1;
static TaskHandle_t tcp_task_handle = NULL;
static TaskHandle_t tcp_send_task_handle = NULL;
static TaskHandle_t tcp_recv_cmd_task_handle = NULL;

static QueueHandle_t tcp_command_queue = NULL;

// 指令定义
typedef enum {
    CMD_UNKNOWN = 0,
    CMD_START,
    CMD_STOP,
    CMD_ACCELARATE,
    CMD_DECELARATE,
} tcp_command_t;

// 指令处理函数原型
void tcp_send_data_task(void *pvParameters);
void tcp_recv_cmd_task(void *pvParameters);
void tcp_process_command(tcp_command_t cmd);

void tcp_server_task(void *pvParameters) {
    ESP_LOGI("TCP", "Task started");
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t sin_size;
    char recv_buf[1024];
    int recv_len;
    
    // 创建TCP套接字
    if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ESP_LOGE("TCP", "Failed to create socket");
        vTaskDelete(NULL);
        return;
    }
    
    // 设置套接字选项，允许地址重用
    int opt = 1;
    setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 绑定服务器地址和端口
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3333);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sock_listen, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE("TCP", "Failed to bind");
        close(sock_listen);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI("TCP", "Socket bound to port 3333");
    
    // 开始监听
    if (listen(sock_listen, 5) < 0) {
        ESP_LOGE("TCP", "Failed to listen");
        close(sock_listen);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI("TCP", "TCP server started on port 3333");
    
    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        
        // 接受客户端连接
        if ((sock_client = accept(sock_listen, (struct sockaddr *)&client_addr, &sin_size)) < 0) {
            ESP_LOGE("TCP", "Failed to accept");
            continue;
        }
        
        ESP_LOGI("TCP", "Client connected: %s", inet_ntoa(client_addr.sin_addr));
        
        // 创建数据发送任务
        if (tcp_send_task_handle == NULL) {
            xTaskCreate(tcp_send_data_task, "tcp_send", 4096, NULL, 1, &tcp_send_task_handle);  
        }
        
        // 接收数据
        while (1) {
            recv_len = recv(sock_client, recv_buf, sizeof(recv_buf) - 1, 0);    // 如果上位机没有发送数据，recv 会阻塞
            
            if (recv_len <= 0) {    // 客户端已经断开连接或发生了错误
                ESP_LOGI("TCP", "Client disconnected");
                close(sock_client);
                sock_client = -1;
                
                // 停止数据发送任务
                if (tcp_send_task_handle != NULL) {
                    vTaskDelete(tcp_send_task_handle);
                    tcp_send_task_handle = NULL;
                }
                break;
            }
            
            recv_buf[recv_len] = '\0';
            ESP_LOGI("TCP", "Received command: %s", recv_buf);
            
            // 解析指令并发送到队列
            tcp_command_t cmd = CMD_UNKNOWN;
            if (strcmp(recv_buf, "START") == 0) {
                cmd = CMD_START;
            } else if (strcmp(recv_buf, "STOP") == 0) {
                cmd = CMD_STOP;
            } else if (strcmp(recv_buf, "ACCELARATE") == 0) {
                cmd = CMD_ACCELARATE;
            } else if (strcmp(recv_buf, "DECELARATE") == 0) {
                cmd = CMD_DECELARATE;
            }

            if (cmd != CMD_UNKNOWN) {
                if (xQueueSend(tcp_command_queue, &cmd, 0) != pdPASS) {
                    ESP_LOGE("TCP", "Failed to send command to queue");
                }
            } else {
                ESP_LOGE("TCP", "Unknown command received: %s", recv_buf);
            }
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}


void tcp_recv_cmd_task(void *pvParameters){
    tcp_command_t current_cmd;

    while (1) {
        if (xQueueReceive(tcp_command_queue, &current_cmd, portMAX_DELAY) == pdPASS) {
            tcp_process_command(current_cmd);
        }
    }
}
// 指令处理函数
void tcp_process_command(tcp_command_t cmd) {    
    switch (cmd) {
        case CMD_START:
            if (current_spd != 0) {
                printf("电机已启动，当前转速：%d RPM\n", current_spd);
            } else {
                current_status = STATE_ACCELERATING;
                target_spd = MAX_RPM;
                printf("电机启动，开始加速...\n");
            }
            break;

        case CMD_STOP:
            current_status = STATE_STOPPED;
            printf("电机停止...\n");
            break;

        case CMD_ACCELARATE:
            // TODO
            break;

        case CMD_DECELARATE:
            // TODO
            break;

        default:
            printf("Invalid command received\n");
            break;
    }
}

// 数据发送任务
void tcp_send_data_task(void *pvParameters) {
    ESP_LOGI("TCP_SEND", "Data send task started");
  
    while (1) {
        if (sock_client != -1) {
            char data_msg[64];
            snprintf(data_msg, sizeof(data_msg), "Current speed:%d\n", current_spd);
            
            if (send(sock_client, data_msg, strlen(data_msg), 0) < 0) {
                ESP_LOGE("TCP_SEND", "Send failed");
                break;
            }
        }
        
        // 每100ms发送一次数据
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void test_wifi_event_cb(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI("WIFI", "retry to connect to the AP");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("WIFI", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        printf("got ip success! IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));
        
        // 获取IP后启动TCP服务器任务
        if (tcp_task_handle == NULL) {
            printf("Starting TCP server task...\n");
            xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 3, &tcp_task_handle);
        }
    }
};

void init_wifi(void)
{
    tcp_command_queue = xQueueCreate(10, sizeof(tcp_command_t));
    xTaskCreate(tcp_recv_cmd_task, "motor_command", 4096, NULL, 2, &tcp_recv_cmd_task_handle);

    // 初始化
    nvs_flash_init();   // FLash用于持久保存配置信息​​，如WiFi名称和密码
    esp_netif_init();   // 提供了一个​​统一的接口​​来管理和配置不同类型的物理或虚拟网络接口
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    // 注册事件回调
    esp_event_handler_instance_t test_wifi_event_handler;
    esp_event_handler_instance_register(WIFI_EVENT,
                                            ESP_EVENT_ANY_ID,
                                            test_wifi_event_cb,
                                            NULL,
                                            &test_wifi_event_handler);
    esp_event_handler_instance_t test_ip_event_handler;
    esp_event_handler_instance_register(IP_EVENT,
                                            IP_EVENT_STA_GOT_IP,
                                            test_wifi_event_cb,
                                            NULL,
                                            &test_ip_event_handler);

    // 配置WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            // .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            // .pmf_cfg = {
            //     .capable = true,
            //     .required = false
            // },
        },
    };
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}
