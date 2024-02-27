#include "app_http_server.h"
#include "esp_mac.h"
#include "gpio.h"
#include "i2c_lcd.h"
static char * TAG = "Nhom1";
static EventGroupHandle_t s_wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;
static const int WIFI_FAIL_BIT = BIT1;
static int s_retry_num = 0; 
#define EXAMPLE_ESP_WIFI_SSID      "Olala2"
#define EXAMPLE_ESP_WIFI_PASS      "cabimo123"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = 0;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 21,
        .scl_io_num = 22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        // doc ip ve
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void app_config()
{    
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init()); //init network interface
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // init loop event (kĩ thuật dk 1 loạt sk, khi sk diễn ra nó sẽ kéo theo những sk khác)
    esp_netif_create_default_wifi_sta(); // khởi tạo wifi station
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); 
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
// tạo sk sẽ diễn ra nếu nhận dc kết nối sẽ back lại ip, nếu k nhận dc thì sẽ check lại 5 lần
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    // config wifi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); 
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start() );
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    // đợi xem là kết nối fail hay dc
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);
    // xử lý vs thông tin nhận được
    if (bits & WIFI_CONNECTED_BIT) {
        // thành công lên web, gửi các thông tin
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
                start_webserver();
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

void app_main(){
    // Kiem tra vung nho cua esp
    Set_Pin_Output(4);
    Set_Pin_Output(5);

    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    lcd_init();
    lcd_clear();
    esp_err_t rett = nvs_flash_init();
    if (rett == ESP_ERR_NVS_NO_FREE_PAGES || rett == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      rett = nvs_flash_init();
    }
    ESP_ERROR_CHECK(rett);
    app_config();
}




