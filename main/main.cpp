#include <stdio.h>
#include <esp_log.h>
#include <bsp/esp32_p4_wifi6_touch_lcd_7b.h>
#include <ui.h>
#include <screens.h>
#include <actions.h>
#include <freertos/FreeRTOS.h>
#include <wifi.h>
#include <esp_hosted.h>

static const char *TAG="MAIN";
extern objects_t objects;

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

void action_wifi_scan(lv_event_t *e)  {
    ESP_LOGI(TAG, "Action scan");
    lv_obj_add_state(objects.scan, LV_STATE_DISABLED);
    wifiScan();
}

void handleMainEvents(uint32_t timeout) {
    mainBits=xEventGroupWaitBits(mainEventGroup, 0x00ffffff, pdTRUE, pdFALSE, timeout);
}

void handleWifiEvents(uint32_t timeout) {
    wifiBits=xEventGroupWaitBits(wifiEventGroup, 0x00ffffff, pdTRUE, pdFALSE, timeout);
    if (wifiBits&WIFI_CONNECTED_BIT)  {
    }
    if (wifiBits&WIFI_FAIL_BIT) {
    }
    if (wifiBits&WIFI_SCAN_DONE)    {
        ESP_LOGI(TAG, "Scan done");
        lv_obj_clear_state(objects.scan, LV_STATE_DISABLED);
        ESP_LOGI(TAG, "Creating scan list");
        uint16_t apNum=wifiGetAPNum();
        int32_t row=0;
        uint16_t width=lv_obj_get_width(objects.sta_scan_list);
        if (apNum>0) lv_table_set_row_cnt(objects.sta_scan_list, apNum);
        lv_table_set_col_cnt(objects.sta_scan_list, 1);
        lv_table_set_col_width(objects.sta_scan_list, 0, width-4);//-48);
        //lv_table_set_col_width(objects.sta_scan_list, 1, 48);
        while (apNum>0) {
            char *ssid=(char *)wifiGetAPRecord()->ssid;
            ESP_LOGI(TAG, "Found AP: %s", ssid);
            lv_table_set_cell_value(objects.sta_scan_list, row, 0, ssid);
            row++;
            apNum--;
        }
    }
}

void uiRefresh()    {
    if (bsp_display_lock(0)) {
        ui_tick();
        bsp_display_unlock();
    }
}

void action_select_ssid(lv_event_t *e) {
    uint16_t row, col;
    const char *val;
    lv_obj_t *obj=lv_event_get_target(e);
    lv_table_get_selected_cell(obj, &row, &col);
    ESP_LOGI(TAG, "Selected %d, %d", row, col);
    val=lv_table_get_cell_value(obj, row, col);
    lv_textarea_set_text(objects.ssid_text_area, val);
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
