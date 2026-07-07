#ifdef __cplusplus
extern "C"  {
#endif

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

void wifiInit(void *args);

void wifiSetupConnection(const char *_ssid, const char *_password, const char *_bssid);
uint32_t wifiGetStatus();
void wifiSetStatus(uint32_t status);
void wifiClearStatus(uint32_t status);



#ifdef __cplusplus
}
#endif