#ifndef __APP_HTTP_SERVER_H
#define __APP_HTTP_SERVER_H
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#define EX_UART_NUM UART_NUM_2
#define PATTERN_CHR_NUM    (3)         /*!< Set the number of consecutivswacxddddddddddde and identical characters received by receiver which defines a UART pattern*/
#define TX_EN 18
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

typedef void (*http_server_handle)(char *data, int len);
void start_webserver();
void stop_webserver(void);
void http_send_response(char *data, int len);
#endif