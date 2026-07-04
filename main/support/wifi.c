#include <wifi.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_wifi_remote.h>
#include <nvs_flash.h>
#include <esp_log.h>

static const char *TAG="WIFI";

#define MAXIMUM_RETRY 3

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static EventGroupHandle_t wifiEventGroup;
static int retryCnt;
wifi_ap_record_t    record;
uint16_t apNum=32;

static void event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
    if (event_base==WIFI_EVENT)    {
        switch (event_id)   {
        case WIFI_EVENT_SCAN_DONE:
            ESP_LOGI(TAG, "Wifi scan done");
            esp_wifi_scan_get_ap_num(&apNum);
            while (apNum>0) {
                esp_wifi_scan_get_ap_record(&record);
                ESP_LOGI(TAG, "AP: %s, power:%d", record.ssid, record.rssi);
                apNum--;
            }
            break;
        case WIFI_EVENT_STA_START: 
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            if (retryCnt<MAXIMUM_RETRY) {
                ESP_ERROR_CHECK(esp_wifi_connect());
                retryCnt++;
                ESP_LOGI(TAG, "retry to connect to the AP");
            } else {
                xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
            }
            ESP_LOGI(TAG,"connect to the AP fail");
            break;
        }
    }   else if (event_base==IP_EVENT)  {
        switch (event_id)   {
            case    IP_EVENT_STA_GOT_IP:
                ip_event_got_ip_t *event=(ip_event_got_ip_t*)event_data;
                ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
                retryCnt=0;
                xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
            break;
        }
    }
}

void wifiInit(void *args) {
    esp_err_t ret=nvs_flash_init();
    if (ret==ESP_ERR_NVS_NO_FREE_PAGES||ret==ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret=nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifiEventGroup=xEventGroupCreate();
    ESP_LOGI(TAG, "Netif init");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg=WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

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

     wifi_config_t wifi_config = {
        .sta = {
            .ssid = "Halo",
            .password = "amiga4000",
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    while (1)   {
        EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
                WIFI_CONNECTED_BIT|WIFI_FAIL_BIT,
                pdFALSE,
                pdFALSE,
                portMAX_DELAY);

        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "connected to ap");
            xEventGroupClearBits(wifiEventGroup, WIFI_CONNECTED_BIT);
            esp_wifi_scan_start(NULL, false);
        } else if (bits & WIFI_FAIL_BIT) {
            ESP_LOGI(TAG, "Failed to connect");
            xEventGroupClearBits(wifiEventGroup, WIFI_FAIL_BIT);
        } else {
            xEventGroupClearBits(wifiEventGroup, bits);
            ESP_LOGE(TAG, "UNEXPECTED EVENT");
        }
    }
}