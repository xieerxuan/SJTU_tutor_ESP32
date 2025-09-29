#ifndef WIFI_H
#define WIFI_H

#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "lwip/sockets.h"

#define EXAMPLE_ESP_WIFI_SSID      "B118"
#define EXAMPLE_ESP_WIFI_PASS      "lcme2018"


void init_wifi(void);



#endif // WIFI_H