#include <wifi.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_wifi_remote.h>
#include <nvs_flash.h>
#include <esp_log.h>

static const char *TAG="WIFI";
static TaskHandle_t mainTask;
static SemaphoreHandle_t taskSemaphore, stateSemaphore;

enum WIFITASK_STATES {WIFI_INIT=0, WIFI_CONNECTING, WIFI_CONNECTED, WIFI_RUNNING, WIFI_DISCONNECTED, WIFI_SCANNING};
static uint32_t wifiTaskState=WIFI_INIT, wifiNewTaskState=WIFI_INIT;

#define WS_STA_READY_BIT BIT0
#define WS_CONNECTED_BIT BIT1
#define WS_CONNECTING_BIT BIT2
static uint32_t wifiStatus;

#define MAXIMUM_RETRY 3

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define WIFI_STA_READY BIT2
#define WIFI_REQUEST_CONNECT BIT31

static EventGroupHandle_t wifiEventGroup;
static int retryCnt;
static wifi_ap_record_t    record;
static uint16_t apNum=32;

wifi_config_t wifiConfig = {
    .sta={
        .ssid={0},
        .password={0},
    }
};

#define WC_VALID BIT0
static int32_t configFlags=0;

static void wifiChangeState(uint32_t newState)   {
    ESP_LOGI(TAG, "Changing state to %d", newState);
    xSemaphoreTake(stateSemaphore, portMAX_DELAY);
    wifiNewTaskState=newState;
    xSemaphoreGive(stateSemaphore);
}

static void updateState()   {
    ESP_LOGI(TAG, "Update state to %d", wifiNewTaskState);
    xSemaphoreTake(stateSemaphore, portMAX_DELAY);
    wifiTaskState=wifiNewTaskState;
    xSemaphoreGive(stateSemaphore);
}

void wifiSetupConnection(const char *_ssid, const char *_password, const char *_bssid)  {
    strcpy((char *)&wifiConfig.sta.ssid, _ssid);
    strcpy((char *)&wifiConfig.sta.password, _password);
    configFlags|=WC_VALID;
}

uint32_t wifiGetStatus()    {
    uint32_t ret;
    xSemaphoreTake(taskSemaphore, portMAX_DELAY);
    ret=wifiStatus;
    xSemaphoreGive(taskSemaphore);
    return ret;
}

void wifiSetStatus(uint32_t status)    {
    xSemaphoreTake(taskSemaphore, portMAX_DELAY);
    wifiStatus|=status;
    xSemaphoreGive(taskSemaphore);
}

void wifiClearStatus(uint32_t status)    {
    xSemaphoreTake(taskSemaphore, portMAX_DELAY);
    wifiStatus&=~status;
    xSemaphoreGive(taskSemaphore);
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
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
            xEventGroupSetBits(wifiEventGroup, WIFI_STA_READY);
            //ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            wifiClearStatus(WS_CONNECTED_BIT);
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
    mainTask=(TaskHandle_t)args;
    taskSemaphore=xSemaphoreCreateMutex();
    stateSemaphore=xSemaphoreCreateMutex();
    esp_err_t ret=nvs_flash_init();
    if (ret==ESP_ERR_NVS_NO_FREE_PAGES||ret==ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret=nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifiEventGroup=xEventGroupCreate();
    wifiTaskState=WIFI_INIT;
    ESP_LOGI(TAG, "Netif init");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg=WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    while (1)   {
        EventBits_t bits = xEventGroupWaitBits(wifiEventGroup, 0xFFFFFFFF, pdFALSE, pdFALSE, portMAX_DELAY);

        switch (wifiTaskState)  {
            case WIFI_INIT:
                wifiHandleInit(bits);
            break;
            case WIFI_CONNECTING:
                wifiHandleConnecting(bits);
            break;
            case WIFI_CONNECTED:
                wifiHandleConnected(bits);
            break;
            case WIFI_DISCONNECTED:
                wifiHandleDisconnected(bits);
            break;
            case WIFI_SCANNING:
                wifiHandleScanning(bits);
            break;
            case WIFI_RUNNING:
                wifiHandleRunning(bits);
            break;
            default:
                ESP_LOGE(TAG, "Error: state not supported");
            break;
        }
        
        updateState();

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

void wifiHandleInit(EventBits_t bits)   {
    if (bits&WIFI_STA_READY)   {
        if (configFlags&WC_VALID)   {
            wifiChangeState(WIFI_CONNECTING);
        }
    }
}

void wifiHandleConnecting(EventBits_t bits) {
    esp_err_t ret;
    uint32_t status=wifiGetStatus();
    if ((!(status&WS_CONNECTING_BIT)) || (!(status&WS_CONNECTED_BIT))) {
        ret=esp_wifi_connect();
    }
    if (bits&WIFI_CONNECTED_BIT) {
        wifiChangeState(WIFI_CONNECTED);
    }
}

void wifiHandleConnected(EventBits_t bits)  {

}

void wifiHandleDisconnected(EventBits_t bits)   {

}

void wifiHandleScanning(EventBits_t bits)   {

}

void wifiHandleRunning(EventBits_t bits)    {

}