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

void wifiHandleInit(EventBits_t bits);
void wifiHandleConnecting(EventBits_t bits);
void wifiHandleConnected(EventBits_t bits);
void wifiHandleDisconnected(EventBits_t bits);
void wifiHandleScanning(EventBits_t bits);
void wifiHandleRunning(EventBits_t bits);


#ifdef __cplusplus
}
#endif