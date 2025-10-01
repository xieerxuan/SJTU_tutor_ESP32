#include <stdio.h>
#include "wifi.h"
static int sock_listen = -1;
static int sock_client = -1;
static TaskHandle_t tcp_task_handle = NULL;

// void tcp_send_task(void *pvParameters)
// {
//     const char *esp32_ip = "192.168.1.100";
//     const int esp32_port = 3333;
    
//     while (1) {
//         gui_command_t cmd;
//         if (xQueueReceive(get_gui_command_queue(), &cmd, portMAX_DELAY) == pdPASS) {
//             if (cmd == GUI_CMD_EXIT) {
//                 break;
//             }
            
//             const char *message = NULL;
//             if (cmd == GUI_CMD_START) {
//                 message = "start";
//             } else if (cmd == GUI_CMD_STOP) {
//                 message = "stop";
//             }
            
//             if (message) {
//                 // 这里添加TCP发送代码
//                 printf("Sending command: %s to %s:%d\n", message, esp32_ip, esp32_port);
//                 // send_tcp_command(esp32_ip, esp32_port, message);
//             }
//         }
//     }
    
//     vTaskDelete(NULL);
// }

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
    
    // 绑定服务器地址和端口
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3333);  // 使用3333端口
    server_addr.sin_addr.s_addr = INADDR_ANY;
    ESP_LOGI("TCP", "Socket bound to port 3333");
    
    if (bind(sock_listen, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE("TCP", "Failed to bind");
        close(sock_listen);
        vTaskDelete(NULL);
        return;
    }
    
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
        
        // 接收数据
        while (1) {
            recv_len = recv(sock_client, recv_buf, sizeof(recv_buf) - 1, 0);
            
            if (recv_len <= 0) {
                ESP_LOGI("TCP", "Client disconnected");
                close(sock_client);
                sock_client = -1;
                break;
            }
            
            // 确保字符串以null结尾
            recv_buf[recv_len] = '\0';
            
            // 打印接收到的指令
            ESP_LOGI("TCP", "Received command: %s", recv_buf);
            printf("Received: %s\n", recv_buf);
            
            // 这里可以添加指令处理逻辑
            // 例如：if (strcmp(recv_buf, "LED_ON") == 0) { ... }
            
            // 可选：发送响应
            // send(sock_client, "OK", 2, 0);
        }
    }
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
            xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 5, &tcp_task_handle);
        }
    }
};

void init_wifi(void)
{
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
