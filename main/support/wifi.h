#ifdef __cplusplus
extern "C"  {
#endif

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>

#define MAXIMUM_RETRY 3

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define WIFI_SCAN_DONE BIT2

void wifiInit(EventGroupHandle_t _wifiEventGroup);

void wifiSetupConnection(const char *_ssid, const char *_password, const char *_bssid);
uint16_t wifiGetAPNum();
wifi_ap_record_t *wifiGetAPRecord();
void wifiScan();

#ifdef __cplusplus
}
#endif