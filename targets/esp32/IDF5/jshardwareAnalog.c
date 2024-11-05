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
 * Contains ESP32 board specific functions.
 * ----------------------------------------------------------------------------
 */

#include "jsinteractive.h"
#include "jshardwareAnalog.h"
#include "driver/adc.h"
#include "driver/dac.h"
#include "esp_log.h"

#define TAG "jshardwareAnalog"  // ESP-IDF log tag for easier debugging

#if CONFIG_IDF_TARGET_ESP32
    // Configuration specific to ESP32
#elif CONFIG_IDF_TARGET_ESP32C3
    typedef enum { DAC_CHAN_0 = 0, DAC_CHAN_1 = 1 } dac_channel_t;
#elif CONFIG_IDF_TARGET_ESP32S3
    typedef enum { DAC_CHAN_0 = 0, DAC_CHAN_1 = 1 } dac_channel_t;
#else
    #error "Unsupported ESP32 variant"
#endif

/**
 * Initialize the ADC module for a specific group.
 * Placeholder for future ESP-IDF 5.2.2 setup requirements.
 *
 * Relevant Example:
 * - ADC Continuous Read: https://github.com/espressif/esp-idf/blob/v5.2/examples/peripherals/adc/continuous_read/main/continuous_read_main.c
 */
void initADC(int ADCgroup) {
    jsiConsolePrintf("jshardwareAnalog.h - initADC: Initializing ADC group %d\n", ADCgroup);
    ESP_LOGI(TAG, "initADC: Configuring ADC for group %d", ADCgroup);
    
    // TODO: Add ESP-IDF 5.2.2 specific ADC configuration here
    // Example: Configure ADC width and attenuation based on ADCgroup
    
    // Error handling example
    esp_err_t err = ESP_OK; // Replace with actual setup function
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ADC: %s", esp_err_to_name(err));
    }
}

/**
 * Reads the ADC value from the specified pin.
 * Adds detailed logging for debugging.
 *
 * Relevant Example:
 * - ADC Oneshot Read: https://github.com/espressif/esp-idf/blob/v5.2/examples/peripherals/adc/oneshot_read/main/oneshot_read_main.c
 */
int readADC(Pin pin) {
    jsiConsolePrintf("jshardwareAnalog.h - readADC: Reading ADC value from pin %d\n", pin);
    ESP_LOGI(TAG, "readADC: Reading ADC value on pin %d", pin);

    // TODO: Replace with actual ADC reading logic
    int adc_value = 0;

    // Error handling for invalid ADC read
    if (adc_value < 0) {
        ESP_LOGW(TAG, "readADC: Invalid ADC read on pin %d", pin);
    }

    return adc_value;
}

/**
 * Sets the range for the ADC on the specified pin.
 * Uses ESP-IDF functions to ensure valid configuration.
 *
 * Relevant Example:
 * - ADC Continuous Read: https://github.com/espressif/esp-idf/blob/v5.2/examples/peripherals/adc/continuous_read/main/continuous_read_main.c
 */
void rangeADC(Pin pin, int range) {
    jsiConsolePrintf("jshardwareAnalog.h - rangeADC: Setting range %d on pin %d\n", range, pin);
    ESP_LOGI(TAG, "rangeADC: Configuring ADC range %d on pin %d", range, pin);

    // TODO: Implement range setting, potentially using attenuation levels
}

/**
 * Writes a value to the DAC on the specified pin.
 *
 * Relevant Example:
 * - DAC Oneshot Output: https://github.com/espressif/esp-idf/blob/v5.2/examples/peripherals/dac/dac_oneshot/main/dac_oneshot_example_main.c
 */
void writeDAC(Pin pin, uint8_t value) {
    jsiConsolePrintf("jshardwareAnalog.h - writeDAC: Writing value %d to DAC on pin %d\n", value, pin);
    ESP_LOGI(TAG, "writeDAC: Writing value %d to DAC on pin %d", value, pin);

    // TODO: Replace with ESP-IDF DAC write function for pin and value
}

/**
 * Resets the ADC to default settings.
 *
 * Relevant Example:
 * - ADC Continuous Read: https://github.com/espressif/esp-idf/blob/v5.2/examples/peripherals/adc/continuous_read/main/continuous_read_main.c
 */
void ADCReset() {
    jsiConsolePrintf("jshardwareAnalog.h - ADCReset: Resetting ADC\n");
    ESP_LOGI(TAG, "ADCReset: Resetting ADC to default settings");

    // TODO: Implement reset logic for ADC, clearing any custom configurations
}



#if original
#include "jshardwareAnalog.h"
#include "driver/adc.h"
#if CONFIG_IDF_TARGET_ESP32
	#include "driver/dac.h"
#elif CONFIG_IDF_TARGET_ESP32C3
	typedef enum { DAC_CHAN_0=0 , DAC_CHAN_1=1 } dac_channel_t;
#elif CONFIG_IDF_TARGET_ESP32S3
	typedef enum { DAC_CHAN_0=0 , DAC_CHAN_1=1 } dac_channel_t;
#else
	#error Not an ESP32 or ESP32-S3
#endif

#include <stdio.h>

#define adc_channel_max 8

adc_atten_t adc_channel[8];

adc1_channel_t pinToAdcChannel(Pin pin){
  adc1_channel_t channel;
  if (pinInfo[pin].analog == JSH_ANALOG_NONE)
    return -1;
  switch(pinInfo[pin].analog & JSH_MASK_ANALOG_CH){
    case 0: channel = ADC1_CHANNEL_0; break;
    case 1: channel = ADC1_CHANNEL_1; break;
    case 2: channel = ADC1_CHANNEL_2; break;
    case 3: channel = ADC1_CHANNEL_3; break;
    case 4: channel = ADC1_CHANNEL_4; break;
#ifndef CONFIG_IDF_TARGET_ESP32C3
    case 5: channel = ADC1_CHANNEL_5; break;
    case 6: channel = ADC1_CHANNEL_6; break;
    case 7: channel = ADC1_CHANNEL_7; break;
#endif
    default: channel = -1; break;
  }
  return channel;
}
adc_atten_t rangeToAdcAtten(int range){
  adc_atten_t atten;
  switch (range){
#if ESP_IDF_VERSION_MAJOR>=4
	case 1000: atten = ADC_ATTEN_DB_0; break;
	case 1340: atten = ADC_ATTEN_DB_2_5; break;
	case 2000: atten = ADC_ATTEN_DB_6; break;
	case 3600: atten = ADC_ATTEN_DB_11; break;
	default: atten = ADC_ATTEN_DB_11; break;
#else
	case 1000: atten = ADC_ATTEN_0db; break;
	case 1340: atten = ADC_ATTEN_2_5db; break;
	case 2000: atten = ADC_ATTEN_6db; break;
	case 3600: atten = ADC_ATTEN_11db; break;
	default: atten = ADC_ATTEN_11db; break;
#endif
  }
  return atten;
}
int pinToAdcChannelIdx(Pin pin){
  if (pinInfo[pin].analog == JSH_ANALOG_NONE)
    return -1;
  return pinInfo[pin].analog & JSH_MASK_ANALOG_CH;
}

dac_channel_t pinToDacChannel(Pin pin){
#if CONFIG_IDF_TARGET_ESP32
  dac_channel_t channel;
  switch(pin){
    case 25: channel = DAC_CHANNEL_1; break;
    case 26: channel = DAC_CHANNEL_2; break;
    default: channel = -1; break;
  }
  return channel;
#elif CONFIG_IDF_TARGET_ESP32C3
  jsExceptionHere(JSET_ERROR, "not implemented\n");
  return 0;
#elif CONFIG_IDF_TARGET_ESP32S3
  jsExceptionHere(JSET_ERROR, "not implemented\n");
  return 0;
#else
	#error Not an ESP32 or ESP32-S3
#endif
}

void ADCReset(){
  initADC(1);
}
void initADC(int ADCgroup){
  switch(ADCgroup){
  case 1:
#if ESP_IDF_VERSION_MAJOR>=4
    adc1_config_width(ADC_WIDTH_BIT_12);
#else
    adc1_config_width(ADC_WIDTH_12Bit);
#endif
    for(int i = 0; i < adc_channel_max; i++) {
#if ESP_IDF_VERSION_MAJOR>=4
      adc_channel[i] = ADC_ATTEN_DB_11;
#else
      adc_channel[i] = ADC_ATTEN_11db;
#endif
     }
    break;
  case 2:
    jsExceptionHere(JSET_ERROR, "Not implemented");
    break;
  case 3:
    jsExceptionHere(JSET_ERROR, "Not implemented");
    break;
  default:
    jsExceptionHere(JSET_ERROR, "Out of range");
  break;
  }
}

void rangeADC(Pin pin,int range){
  int idx,atten;
  idx = pinToAdcChannelIdx(pin);
  printf("idx:%d\n",idx);
  if(idx >= 0){
    adc_channel[idx] = rangeToAdcAtten(range);
    printf("Atten:%d \n",adc_channel[idx]);
  }
}

int readADC(Pin pin){
  adc1_channel_t channel; int value;
  channel = pinToAdcChannel(pin);
  if(channel >= 0) {
    adc1_config_channel_atten(channel,adc_channel[pinToAdcChannelIdx(pin)]);
#if ESP_IDF_VERSION_MAJOR>=4
	  // ESP_IDF 4.x - int adc1_get_voltage(adc1_channel_t channel)    //Deprecated. Use adc1_get_raw() instead
	  int value=adc1_get_raw(channel);
#if CONFIG_IDF_TARGET_ESP32C3
    // we need to call it twice to get a decent value on the C3 for some reason!
    // https://forum.espruino.com/conversations/395499/?offset=100#17482163
    value=adc1_get_raw(channel);
#endif
#else
	  value = adc1_get_voltage(channel);
#endif
    return value;
  } else return -1;
}

void writeDAC(Pin pin,uint8_t value){
  dac_channel_t channel;
  if(value > 255){
    jsExceptionHere(JSET_ERROR, "Not implemented, only 8 bit supported");
    return;
  }
#if CONFIG_IDF_TARGET_ESP32
  channel = pinToDacChannel(pin);
#if ESP_IDF_VERSION_MAJOR>=4
  if(channel >= 0) dac_output_voltage(channel, value);
#else
  if(channel >= 0) dac_out_voltage(channel, value);
#endif
#elif CONFIG_IDF_TARGET_ESP32C3
  jsExceptionHere(JSET_ERROR, "not implemented\n");
#elif CONFIG_IDF_TARGET_ESP32S3
  jsExceptionHere(JSET_ERROR, "not implemented\n");
#else
	#error Not an ESP32 or ESP32-S3
#endif
}

#endif // original

