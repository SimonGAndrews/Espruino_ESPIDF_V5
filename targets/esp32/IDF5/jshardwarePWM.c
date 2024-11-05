/*
 * This file is designed to support PWM functions in Espruino,
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
#include "jshardwarePWM.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define TAG "jshardwarePWM"  // ESP-IDF log tag for debugging

/**
 * Initializes the PWM module.
 *
 * Relevant Example:
 * - LEDC PWM Example: https://github.com/espressif/esp-idf/tree/v5.2/examples/peripherals/ledc
 */
void PWMInit() {
    jsiConsolePrintf("jshardwarePWM.h - PWMInit: Initializing PWM\n");
    ESP_LOGI(TAG, "Initializing PWM");

    // Initialize PWM timers and channels as required
    // Configure frequency, timer bit width, and other settings
    esp_err_t err = ledc_fade_func_install(0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LEDC fade function: %s", esp_err_to_name(err));
    }
}

/**
 * Writes a PWM signal to the specified pin with a given value and frequency.
 *
 * Relevant Example:
 * - LEDC PWM Example: https://github.com/espressif/esp-idf/tree/v5.2/examples/peripherals/ledc
 */
void writePWM(Pin pin, uint16_t value, int freq) {
    jsiConsolePrintf("jshardwarePWM.h - writePWM: Writing PWM on pin %d, value %d, frequency %d\n", pin, value, freq);
    ESP_LOGI(TAG, "Configuring PWM on pin %d with value %d and frequency %d", pin, value, freq);

    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_CHANNEL_0,
        .duty       = value,
        .gpio_num   = pin,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_0
    };

    esp_err_t err = ledc_channel_config(&ledc_channel);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC channel: %s", esp_err_to_name(err));
    }

    err = ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set LEDC frequency: %s", esp_err_to_name(err));
    }
}

/**
 * Sets the PWM value for the specified pin.
 */
void setPWM(Pin pin, uint16_t value) {
    jsiConsolePrintf("jshardwarePWM.h - setPWM: Setting PWM on pin %d with value %d\n", pin, value);
    ESP_LOGI(TAG, "Setting PWM on pin %d with duty cycle %d", pin, value);

    esp_err_t err = ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, value);
    if (err == ESP_OK) {
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    } else {
        ESP_LOGE(TAG, "Failed to set LEDC duty cycle: %s", esp_err_to_name(err));
    }
}



#if original
#include "jsutils.h"

#include "jshardwarePWM.h"
#include "driver/ledc.h"

#include <stdio.h>

#define PWMFreqDefault 5000
#define PWMPinEmpty 111
#define PWMTimerDefault 3

int getTimerIndex(Pin pin,int freq){
  int i;
  for(i = 0; i < PWMFreqMax; i++){
  if(PWMFreqChannels[i].pin == pin) return i;
  }
  return -1;
}
int getFreeTimer(Pin pin,int freq){
  int i;
  i = getTimerIndex(pin,freq);
  if(i >= 0) return i;
  for(i = 0; i < PWMFreqMax; i++){
  if(PWMFreqChannels[i].pin == PWMPinEmpty) return i;
  }
  return -1;
}

int getChannelIndex(Pin pin){
  int i;
  for(i = 0; i < PWMMax; i++){
    if(PWMChannels[i].pin == pin) return i;
  }
  return -1;
}
int getFreeChannel(pin){
  int i;
  i = getChannelIndex(pin);
  if(i >= 0) return i;
  for(i = 0; i < PWMMax; i++){
  if(PWMChannels[i].pin == PWMPinEmpty) return i;
  }
  return -1;
}

void timerConfig(int freq,int timer){
  ledc_timer_config_t PWM_timer = {
#if ESP_IDF_VERSION_MAJOR>=4
    .duty_resolution = LEDC_TIMER_10_BIT,
#else
    .bit_num = LEDC_TIMER_10_BIT,//set timer counter bit number
#endif
#if CONFIG_IDF_TARGET_ESP32
    .freq_hz = freq,//set frequency of pwm
    .speed_mode = LEDC_HIGH_SPEED_MODE,//timer mode,
    .timer_num = timer//timer index
  };
#elif CONFIG_IDF_TARGET_ESP32C3
    .freq_hz = freq,//set frequency of pwm
    .speed_mode = LEDC_LOW_SPEED_MODE,//timer mode,
    .timer_num = timer//timer index
  };
#elif CONFIG_IDF_TARGET_ESP32S3
    .freq_hz = freq,//set frequency of pwm
    .speed_mode = LEDC_LOW_SPEED_MODE,//timer mode,
    .timer_num = timer//timer index
  };
#else
	#error Not an ESP32 or ESP32-S3
#endif
  ledc_timer_config(&PWM_timer);
}

void channelConfig(int timer, int channel, int value, Pin pin){
#if CONFIG_IDF_TARGET_ESP32
  ledc_channel_config_t PWM_channel = {
    .channel = channel,//set LEDC channel 0
    .duty = value,//set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)
    .gpio_num = pin,//GPIO number
    .intr_type = LEDC_INTR_DISABLE,//GPIO INTR TYPE, as an example, we enable fade_end interrupt here.
    .speed_mode = LEDC_HIGH_SPEED_MODE,//set LEDC mode, from ledc_mode_t
    .timer_sel = timer
  };
#elif CONFIG_IDF_TARGET_ESP32C3
  ledc_channel_config_t PWM_channel = {
    .channel = channel,//set LEDC channel 0
    .duty = value,//set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)
    .gpio_num = pin,//GPIO number
    .intr_type = LEDC_INTR_DISABLE,//GPIO INTR TYPE, as an example, we enable fade_end interrupt here.
    .speed_mode = LEDC_LOW_SPEED_MODE,//set LEDC mode, from ledc_mode_t
    .timer_sel = timer
  };
#elif CONFIG_IDF_TARGET_ESP32S3
  ledc_channel_config_t PWM_channel = {
    .channel = channel,//set LEDC channel 0
    .duty = value,//set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)
    .gpio_num = pin,//GPIO number
    .intr_type = LEDC_INTR_DISABLE,//GPIO INTR TYPE, as an example, we enable fade_end interrupt here.
    .speed_mode = LEDC_LOW_SPEED_MODE,//set LEDC mode, from ledc_mode_t
    .timer_sel = timer
  };
#else
	#error Not an ESP32 or ESP32-S3
#endif
  ledc_channel_config(&PWM_channel);
}

void PWMInit(){
  int i;
  timerConfig(PWMFreqDefault,PWMTimerDefault);
  for(i = 0; i < PWMMax; i++) PWMChannels[i].pin = PWMPinEmpty;
  for(i = 0; i < PWMFreqMax; i++) PWMFreqChannels[i].pin = PWMPinEmpty;
}

void writePWM(Pin pin,uint16_t value,int freq){
  int channel; int timer;
  if(freq == PWMFreqDefault || freq == 0){
    channel = getFreeChannel(pin);
    if(channel < 0){jsError("no PWM channel available anymore");}
    else{
    PWMChannels[channel].pin = pin;
    channelConfig(PWMTimerDefault,channel,value,pin);
  }
  }
  else{
  timer = getFreeTimer(pin,freq);
  if(timer < 0){jsError("no PWM channel available anymore");}
  else{
    PWMFreqChannels[timer].pin = pin;
    PWMFreqChannels[timer].freq = freq;
    timerConfig(freq,timer);
    channelConfig(timer,PWMMax + timer,value,pin);
  }
  }
}

void setPWM(Pin pin,uint16_t value){
#if CONFIG_IDF_TARGET_ESP32
  int channel = getChannelIndex(pin);
  if(channel < 0) jsError("pin not assigned to pwm");
  else ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, value);
#elif CONFIG_IDF_TARGET_ESP32C3
	// No DAC implemented on ESP32C3
#elif CONFIG_IDF_TARGET_ESP32S3
	// No DAC implemented on ESP32S3
#else
	#error Not an ESP32 or ESP32-S3
#endif
}

#endif // original

