set(COMPONENT_REQUIRES "freertos" "esp_log")

idf_component_register(SRCS "wifi.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_wifi nvs_flash lwip state_manager)
