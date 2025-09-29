#include <stdio.h>
#include "wifi.h"

void test_wifi_event_cb(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI("WIFI", "retry to connect to the AP");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("WIFI", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        printf("got ip seuccess!\n");
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
                                            IP_EVENT_ETH_GOT_IP,
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
