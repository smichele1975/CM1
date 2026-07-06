#include <stdio.h>
#include <esp_log.h>
#include <bsp/esp32_p4_wifi6_touch_lcd_7b.h>
#include <ui.h>
#include <freertos/FreeRTOS.h>
#include <wifi.h>
#include <esp_hosted.h>

static const char *TAG="MAIN";

static TaskHandle_t wifiTask;

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

EventGroupHandle_t mainEvents;

extern "C" void app_main(void) {
    bsp_spiffs_mount();
    bsp_display_start_with_config(&cfg);
    bsp_display_brightness_set(100);
    bsp_display_lock(-1);
    ui_init();
    bsp_display_unlock();
    xTaskCreatePinnedToCore(wifiInit, "WIFI Task", 4096, xTaskGetCurrentTaskHandle(), 1, &wifiTask, 1);
    while (1)   {
        xEventGroupWaitBits(&mainEvents, 0xFFFFFFFF, pdFALSE, pdFALSE, 0);
        if (bsp_display_lock(0)) {
            ui_tick();
            bsp_display_unlock();
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
