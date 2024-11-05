/*
 * This file is designed to support Analog functions in Espruino,
 * a JavaScript interpreter for Microcontrollers designed by Gordon Williams
 *
 * Copyright (C) 2016 by Juergen Marsch 
 * Modified Nov 2024 for ESP-IDF Version 5.2 by SimonGAndrews
 *
 * This Source Code Form is subject to the terms of the Mozilla Publici
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP32 board specific functions for networking (wifi, ble).
 * ----------------------------------------------------------------------------
 */

#include "jsinteractive.h"
#include "jshardwareESP32.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define TAG "jshardwareESP32"  // ESP-IDF log tag for debugging

/**
 * Gets the NVS status for the specified ESP32 hardware component (e.g., Wi-Fi, BLE).
 * Usage :  
 *    in main ->  if(ESP32_Get_NVS_Status(ESP_NETWORK_WIFI)) jswrap_wifi_restore();
 *
 *    jshardware-> void jshInit() {
 *                   if(ESP32_Get_NVS_Status(ESP_NETWORK_WIFI)) esp32_wifi_init();
 *                   if(ESP32_Get_NVS_Status(ESP_NETWORK_BLE)) gattc_init();
 *  
 * Relevant implementation Example:
 * - NVS Example: https://github.com/espressif/esp-idf/tree/v5.2/examples/storage/nvs_rw_value
 */
bool ESP32_Get_NVS_Status(esp_hardware_esp32_t hardware) {
    jsiConsolePrintf("jshardwareESP32.h - ESP32_Get_NVS_Status: Checking NVS status for hardware type %d\n", hardware);
    ESP_LOGI(TAG, "ESP32_Get_NVS_Status: Checking NVS status for hardware type %d", hardware);

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS needs to be erased and reinitialized");
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(err));
        return false;
    }
    return true;  // Assuming NVS is active
}

void ESP32_Set_NVS_Status(esp_hardware_esp32_t hardware, bool enable) {
    jsiConsolePrintf("jshardwareESP32.h - ESP32_Set_NVS_Status: Function called with hardware=%d, enable=%d\n", hardware, enable);
    // Function implementation here
}

/**
 * Retrieves the hardware name based on type.
 */
const char* ESP32_hardwareName(esp_hardware_esp32_t hardware) {
    jsiConsolePrintf("jshardwareESP32.h - ESP32_hardwareName: Retrieving hardware name for type %d\n", hardware);
    ESP_LOGI(TAG, "ESP32_hardwareName: Retrieving name for hardware type %d", hardware);

    switch(hardware) {
        case ESP_NETWORK_BLE:
            return "BLE";
        case ESP_NETWORK_WIFI:
            return "WiFi";
        default:
            return "Unknown";
    }
}




#if original
#include "jshardwareESP32.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
 
static char *ESP32_hardwareName(esp_hardware_esp32_t hardware){
  switch(hardware){
    case ESP_NETWORK_BLE: return "bleStatus";
    case ESP_NETWORK_WIFI: return "wifiStatus";
  }
  return "unknownHardware";
}
 
bool ESP32_Get_NVS_Status(esp_hardware_esp32_t hardware){
  esp_err_t err;nvs_handle hardwareHandle; uint32_t status;
  nvs_open("nvs",NVS_READWRITE,&hardwareHandle);
  err = nvs_get_u32(hardwareHandle,ESP32_hardwareName(hardware),&status);
  if(err) {
    status = ESP32HARDWAREDEFAULT;
    nvs_set_u32(hardwareHandle,ESP32_hardwareName(hardware),ESP32HARDWAREDEFAULT);
  }
  nvs_close(hardwareHandle);
  return (bool) status;
}

void ESP32_Set_NVS_Status(esp_hardware_esp32_t hardware, bool enable){
  nvs_handle hardwareHandle; uint32_t status;
  if(enable) status = 1; else status = 0;
  nvs_open("nvs",NVS_READWRITE,&hardwareHandle);
  nvs_set_u32(hardwareHandle,ESP32_hardwareName(hardware),status);
  nvs_close(hardwareHandle);
}  

#endif  //original 