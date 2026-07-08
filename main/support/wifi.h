#ifdef __cplusplus
extern "C"  {
#endif

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#define MAXIMUM_RETRY 3

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define WIFI_STA_READY BIT2
#define WIFI_REQUEST_CONNECT BIT31

void wifiInit(EventGroupHandle_t _wifiEventGroup);

void wifiSetupConnection(const char *_ssid, const char *_password, const char *_bssid);

#ifdef __cplusplus
}
#endif