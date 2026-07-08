#include <stdio.h>
#include <esp_log.h>
#include <bsp/esp32_p4_wifi6_touch_lcd_7b.h>
#include <ui.h>
#include <freertos/FreeRTOS.h>
#include <wifi.h>
#include <esp_hosted.h>

static const char *TAG="MAIN";

void handleMainEvents(uint32_t timeout);
void handleWifiEvents(uint32_t timeout);
void uiRefresh();

static bsp_display_cfg_t  cfg={
    .lvgl_port_cfg=ESP_LVGL_PORT_INIT_CONFIG(),
    .buffer_size=1024*60,
    .double_buffer=1,
    .flags={
        .buff_dma=1,
        .buff_spiram=0,
        .sw_rotate=0
    }
};

static EventGroupHandle_t mainEventGroup;
static EventGroupHandle_t wifiEventGroup;
static EventBits_t mainBits, wifiBits;

void handleMainEvents(uint32_t timeout) {
    mainBits=xEventGroupWaitBits(mainEventGroup, 0x00ffffff, pdTRUE, pdFALSE, timeout);
}

void handleWifiEvents(uint32_t timeout) {
    wifiBits=xEventGroupWaitBits(wifiEventGroup, 0x00ffffff, pdTRUE, pdFALSE, timeout);
    if (wifiBits&WIFI_CONNECTED_BIT)  {
    }
    if (wifiBits&WIFI_FAIL_BIT) {
    }
}

void uiRefresh()    {
if (bsp_display_lock(0)) {
    ui_tick();
    bsp_display_unlock();
}

extern "C" void app_main(void) {
    //esp_log_level_set("*", ESP_LOG_NONE);
    mainEventGroup=xEventGroupCreate();
    wifiEventGroup=xEventGroupCreate();
    bsp_spiffs_mount();
    bsp_display_start_with_config(&cfg);
    bsp_display_brightness_set(100);
    bsp_display_lock(-1);
    ui_init();
    bsp_display_unlock();
    wifiSetupConnection("RN11S", "amiga4000", NULL);
    wifiInit(wifiEventGroup);
    while (1)   {
        handleMainEvents(0);
        handleWifiEvents(0);
        uiRefresh();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
