/*
 * This file is designed to support Pulse functions in Espruino,
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
 * Contains ESP32 board specific functions.
 * ----------------------------------------------------------------------------
 */

#include "jsinteractive.h"
#include "jshardwarePulse.h"
#include "driver/rmt.h"
#include "esp_log.h"

#define TAG "jshardwarePulse"  // ESP-IDF log tag for debugging

/**
 * Initializes the RMT module.
 *
 * Relevant Example:
 * - RMT TX/RX Example: https://github.com/espressif/esp-idf/tree/v5.2/examples/peripherals/rmt
 */
void RMTInit() {
    jsiConsolePrintf("jshardwarePulse.h - RMTInit: Initializing RMT\n");
    ESP_LOGI(TAG, "Initializing RMT");

    // Configure and initialize the RMT driver
    esp_err_t err = rmt_driver_install(RMT_CHANNEL_0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install RMT driver: %s", esp_err_to_name(err));
        return;
    }
}

/**
 * Resets the RMT configuration.
 */
void RMTReset() {
    jsiConsolePrintf("jshardwarePulse.h - RMTReset: Resetting RMT\n");
    ESP_LOGI(TAG, "Resetting RMT");

    // Reset RMT configuration and disable channels
    rmt_driver_uninstall(RMT_CHANNEL_0);
}

void sendPulse(Pin pin, bool, int duration) {
    jsiConsolePrintf("jshardwarePulse.h - sendPulse: Function called with pin=%d, duration=%d\n", pin, duration);
    // Function implementation here
}


// CHAT GPT also came up with the following


/**
 * Configures the RMT channel for pulse output.
 */
void RMTInitChannel(Pin pin, bool polarity) {
    jsiConsolePrintf("jshardwarePulse.h - RMTInitChannel: Initializing RMT channel on pin %d\n", pin);
    ESP_LOGI(TAG, "Configuring RMT channel for pin %d with polarity %d", pin, polarity);

    rmt_config_t rmt_cfg = {
        .rmt_mode = RMT_MODE_TX,
        .channel = RMT_CHANNEL_0,
        .gpio_num = pin,
        .clk_div = 80,  // Clock divider for RMT
        .mem_block_num = 1,
        .tx_config = {
            .loop_en = false,
            .carrier_en = polarity,
            .idle_level = RMT_IDLE_LEVEL_LOW,
            .idle_output_en = true,
        }
    };

    esp_err_t err = rmt_config(&rmt_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure RMT: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "RMT channel configured for pin %d", pin);
    }
}

/**
 * Sets the pulse to low state.
 */
void setPulseLow() {
    jsiConsolePrintf("jshardwarePulse.h - setPulseLow: Setting pulse to low\n");
    ESP_LOGI(TAG, "Setting pulse to low");

    // TODO: Implementation for setting pulse low
}

/**
 * Sets the pulse to high state.
 */
void setPulseHigh() {
    jsiConsolePrintf("jshardwarePulse.h - setPulseHigh: Setting pulse to high\n");
    ESP_LOGI(TAG, "Setting pulse to high");

    // TODO: Implementation for setting pulse high
}


#if original
#include "jsutils.h"

#include "jshardwarePulse.h"
#include "driver/rmt.h"

#include <stdio.h>

#define RMTPinEmpty 111

rmt_item32_t items[1];

int getRMTIndex(Pin pin){
  int i;
  for(i = 0; i < RMTChannelMax; i++){
  if(RMTChannels[i].pin == pin) return i;
  }
  return -1;
}
int getFreeRMT(Pin pin){
  for(int i = 0; i < RMTChannelMax; i++){
  if(RMTChannels[i].pin == RMTPinEmpty) {
    RMTChannels[i].pin = pin;
    return i;
  }
  }
  return -1;
}

void RMTReset(){
  for(int i = 0; i < RMTChannelMax; i++){
    if(RMTChannels[i].pin != RMTPinEmpty) rmt_driver_uninstall(i);
  }
}
void RMTInit(){
  int i;
  for(i = 0; i < RMTChannelMax; i++) RMTChannels[i].pin = RMTPinEmpty;
}
int RMTInitChannel(Pin pin, bool pulsePolarity){
  rmt_config_t config;
  int i = getFreeRMT(pin);
  if(i >= 0){
    config.rmt_mode = RMT_MODE_TX;
    config.channel = i;
    config.gpio_num = pin;
    config.mem_block_num = 1;
    config.tx_config.loop_en = 0;
    config.tx_config.carrier_en = 0;
    config.tx_config.idle_output_en = 1;
    if(pulsePolarity) config.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
    else config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
    config.tx_config.carrier_duty_percent = 50;
    config.tx_config.carrier_freq_hz = 10000;
    config.tx_config.carrier_level = 1;
    config.clk_div = 80;
    rmt_config(&config);
    rmt_driver_install(config.channel, 0, 0);
    return i;
  }
  else return -1;
}

void setPulseLow(int duration){
  items[0].duration0 = duration;
  items[0].level0 = 0;
  items[0].duration1 = 10;
  items[0].level1 = 1;
}
rmt_item32_t *setPulseHigh(int duration){
  items[0].duration0 = duration;
  items[0].level0 = 1;
  items[0].duration1 = 10;
  items[0].level1 = 0;
}

//pin to be pulsed. value to be pulsed into the pin. duration in milliseconds to hold the pin.
void sendPulse(Pin pin, bool pulsePolarity, int duration){
  int i;
  i = getRMTIndex(pin);
  if(i < 0) i = RMTInitChannel(pin,pulsePolarity);
  if(i >= 0){
    if(pulsePolarity) setPulseLow(duration);else setPulseHigh(duration);
#if ESP_IDF_VERSION_MAJOR>=5
    rmt_set_gpio(i, RMT_MODE_TX, pin, false); //set pin to rmt, in case that it was reset to GPIO(see jshPinSetValue)
#else
    rmt_set_pin(i, RMT_MODE_TX, pin); //set pin to rmt, in case that it was reset to GPIO(see jshPinSetValue)
#endif
    rmt_write_items(i, items,1,1);
  }
  else printf("all RMT channels in use\n");
  return;
}

#endif // original