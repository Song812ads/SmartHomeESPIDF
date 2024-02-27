#include "app_http_server.h"
#include <esp_http_server.h>
#include "esp_log.h"
#include "gpio.h"
#include "i2c_lcd.h"


static const char *TAG = "app_http_server";
static httpd_handle_t server = NULL;
extern const uint8_t web_start[] asm("_binary_web_html_start");
extern const uint8_t web_end[] asm("_binary_web_html_end");
SemaphoreHandle_t xSemaphore;
QueueHandle_t  xQueue;
QueueHandle_t  uart2_queue;
char convertascii[20];
uint8_t Temp_byte1;
uint8_t Temp_byte2;
uint16_t TEMP;
float Temperature = 0;
typedef struct {
    char* uri;
    httpd_method_t method;
    char* data;
    size_t data_len;
    httpd_req_t* req;
} http_request_t;
volatile int temp_ac = 24;
volatile int ac = 0;

static esp_err_t http_request_handle(httpd_req_t *req) {
    http_request_t request;
    request.uri = req->uri;
    request.method = req->method;
    request.data = NULL;
    request.data_len = 0;
    request.req = req;
    if (req->content_len > 0) {
        request.data = malloc(req->content_len);
        if (request.data) {
            request.data_len = httpd_req_recv(req, request.data, req->content_len);
        }
    }
    if (xQueueSendToBack(xQueue, &request, 0) != pdTRUE) {
        // Failed to send request to queue
        httpd_resp_send_500(req);
        free(request.data);
        return ESP_FAIL;
    }
    httpd_resp_send_chunk(req, NULL, 0); 
    return ESP_OK;
}

// xử lý dữ liệu:
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    /* Increment active connection count */
        if (xSemaphoreTake(xSemaphore, (TickType_t)100) == pdFALSE) {
        /* Semaphore not available, limit reached */
        httpd_resp_send_err(req, HTTPD_405_METHOD_NOT_ALLOWED, "Too many request");
        return ESP_OK;
    }
    httpd_resp_send(req, (const char*)web_start, web_end-web_start);
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
    }
    return ESP_OK;
}
//giao thức nhận dữ liệu lưu vào .handler
static const httpd_uri_t hello = {
    .uri       = "/nhom1",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    .user_ctx  = "GettingStart"
};


static esp_err_t disconnect_handler(httpd_req_t *req)
{
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Disconnect Success");
    xSemaphoreGive(xSemaphore);
    return ESP_OK;
}

/* Register the URI handler for /disconnect */
httpd_uri_t disconnect_uri = {
    .uri      = "/disconnect",
    .method   = HTTP_GET,
    .handler  = disconnect_handler,
    .user_ctx = NULL
};

static httpd_req_t *get_req;
void http_send_response(char *data, int len)
{
    httpd_resp_send(get_req, (const char*)data, len); 
}

static void temp_get_handle(char*data , int datalen)
{
    char buf[100];
    sprintf(buf, "{\"temperature\":\"%.1f\"}",Temperature);
    size_t buf_len = strlen(buf);
    http_send_response(buf, buf_len);   
}

static esp_err_t temp_get_data_handler(httpd_req_t *req)
{
    get_req = req;
    temp_get_handle("temp", 4);
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
    }
    vTaskDelay(20);
    return ESP_OK;
}

static const httpd_uri_t temp = {
    .uri       = "/temp",
    .method    = HTTP_GET,
    .handler   = temp_get_data_handler,
    .user_ctx  = NULL
};


static void sw1_data_handler(char*data, int data_len)
{
    if(strstr(data, "1")){
      gpio_set_level(4,1);
    }
    else{
        gpio_set_level(4,0);
    }
}

static const httpd_uri_t led = {
    .uri       = "/led",
    .method    = HTTP_POST,
    .handler   = http_request_handle,
    .user_ctx  = NULL
};

static void sw2_data_handler(char*data, int data_len)
{
     if(strstr(data, "1")){
      gpio_set_level(5,1);
    }
    else{
        gpio_set_level(5,0);
    }
}

static const httpd_uri_t led1 = {
    .uri       = "/led1",
    .method    = HTTP_POST,
    .handler   = http_request_handle,
    .user_ctx  = NULL
};
void uart_send_command(uint8_t data)
{
    uart_write_bytes(UART_NUM_2, (const char*)&data, 1);
}
static void quat_data_handler(char*data, int data_len)
{
    int fan = 0;
    if(strstr(data, "1")){
       fan = 1;  
    }
    else if(strstr(data, "2")){
        fan = 2;
    }
    else if(strstr(data, "3")){
       fan = 3;  
    }
    else{
        fan = 0;
    }
    uart_send_command(fan);
}
static const httpd_uri_t quat = {
    .uri       = "/quat",
    .method    = HTTP_POST,
    .handler   =  http_request_handle,
    .user_ctx  = NULL
};

static void maylanh_data_handler(char*data, int data_len)
{
    // 
    // lcd_put_cur(0, 0);
    // lcd_send_string("hello %c",data);
    if(strstr(data, "0"))
    {
        ac = 0;
        // lcd_clear();
    }
    else if (strstr(data,"1"))
    {
        // lcd_clear();
        // lcd_put_cur(0, 0);
        // sprintf(convertascii, "air-c = %d%cC", temp_ac, 223);
        // lcd_send_string(convertascii);
        ac = 1;
        ESP_LOGI(TAG,"May lanh mo. Nhiet do: %d",temp_ac);
    }   
    if (ac == 1){
        ESP_LOGI(TAG,"May lanh mo. Nhiet do: %d",temp_ac);
                lcd_clear();
        lcd_put_cur(0, 0);
        sprintf(convertascii, "air-c = %d%cC", temp_ac, 223);
        lcd_send_string(convertascii);}
    if (ac == 0){
        ESP_LOGI(TAG,"May lanh tat");
        lcd_clear();
        }
}


static const httpd_uri_t maylanh = {
    .uri       = "/maylanh",
    .method    = HTTP_POST,
    .handler   =  http_request_handle,
    .user_ctx  = NULL
};

static void temp_maylanh_data_handler(char*data, int data_len)
{
    temp_ac = 10*((*data-48))+(*(data+1)-48);
    if (ac == 1){
        ESP_LOGI(TAG,"May lanh mo. Nhiet do: %d",temp_ac);
                        lcd_clear();
        lcd_put_cur(0, 0);
        sprintf(convertascii, "air-c = %d%cC", temp_ac, 223);
        lcd_send_string(convertascii);
    }
}   


static const httpd_uri_t tempmaylanh = {
    .uri       = "/tempmaylanh",
    .method    = HTTP_POST,
    .handler   =  http_request_handle,
    .user_ctx  = NULL
};


static void http_request_task(void *pvParameters) {
    http_request_t request;
    while (1) {
        if (xQueueReceive(xQueue, &request, portMAX_DELAY)) {
            if (request.method == HTTP_POST)
            {
                if (request.data_len > 0) {  
            if (strcmp(request.uri, "/led") == 0) {
                sw1_data_handler(request.data, request.data_len);
                free(request.data);
            } else if (strcmp(request.uri, "/led1") == 0) {
                sw2_data_handler(request.data, request.data_len);
                free(request.data);
            } else if (strcmp(request.uri, "/quat") == 0) {
                quat_data_handler(request.data, request.data_len);
                free(request.data);
            } else if (strcmp(request.uri, "/maylanh") == 0) {
                maylanh_data_handler(request.data, request.data_len);
                free(request.data);
            }
            else if (strcmp(request.uri, "/tempmaylanh") == 0) {
                temp_maylanh_data_handler(request.data, request.data_len);
                free(request.data);
            }
            }
        }
        }}
    vTaskDelete(NULL);
}

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    for(;;) {
        //Waiting for UART event.zas
        if(xQueueReceive(uart2_queue, (void * )&event, (TickType_t)portMAX_DELAY)) {
            bzero(dtmp, RD_BUF_SIZE);
            switch(event.type) {
                case UART_DATA:
                    uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    int len = event.size; // Length of received data
                    if (len > 0) {

                        Temp_byte1 = dtmp[0];
                        Temp_byte2 = dtmp[1];
                        TEMP = (Temp_byte2<<8)|Temp_byte1;
	                    Temperature = (float)TEMP/16;
                    }             
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart2_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart2_queue);
                    break;
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart rx break");
                    break;
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void start_webserver()
{
    xSemaphore = xSemaphoreCreateCounting(2,2);
    xQueue = xQueueCreate(10, sizeof(http_request_t));
        uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart2_queue, 0);
    uart_param_config(EX_UART_NUM, &uart_config);
    uart_set_pin(EX_UART_NUM, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &led);
        httpd_register_uri_handler(server, &led1);
        httpd_register_uri_handler(server, &temp);
        httpd_register_uri_handler(server, &quat);
        httpd_register_uri_handler(server, &maylanh);
        httpd_register_uri_handler(server, &tempmaylanh);
        httpd_register_uri_handler(server, &disconnect_uri);
        xTaskCreate(http_request_task, "http_handle", 4096, NULL,1,NULL);
        xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 2, NULL);
    }
    else{
        ESP_LOGI(TAG, "Error starting server!");
    }
}

void stop_webserver(void)
{
    httpd_stop(server);
    vSemaphoreDelete(xSemaphore);
    vQueueDelete(xQueue);
    vQueueDelete(uart2_queue);
}
