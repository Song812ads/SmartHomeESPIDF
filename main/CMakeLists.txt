set(pri_req wifi_provisioning nvs_flash esp_eth esp-tls protocomm esp_common esp_netif esp_http_server esp_hw_support)
idf_component_register(SRCS "main.c" "app_http_server.c"  "gpio.c" "i2c_lcd.c" 
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_req})
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
