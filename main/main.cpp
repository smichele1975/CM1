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
    .buffer_size=1024*50*2,
    .double_buffer=1,
    .flags={
        .buff_dma=1,
        .buff_spiram=1,
        .sw_rotate=0
    }
};

esp_hosted_coprocessor_fwver_t  fwVer;

extern "C" void app_main(void) {
    esp_hosted_connect_to_slave();
    esp_hosted_get_coprocessor_fwversion(&fwVer);
    ESP_LOGI(TAG, "Firmware version: %d, %d, %d", fwVer.build, fwVer.major1, fwVer.minor1);
    bsp_spiffs_mount();
    bsp_display_start_with_config(&cfg);
    bsp_display_brightness_set(100);
    bsp_display_lock(-1);
    ui_init();
    bsp_display_unlock();
    xTaskCreatePinnedToCore(wifiInit, "WIFI Task", 4096, NULL, 1, &wifiTask, 1);
    while (1)   {
        vTaskDelay(pdMS_TO_TICKS(10));
        if (bsp_display_lock(0)) {
            ui_tick();
            bsp_display_unlock();
        }
    }
}
