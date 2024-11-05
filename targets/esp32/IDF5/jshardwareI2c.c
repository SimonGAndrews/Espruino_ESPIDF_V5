/*
 * This file is designed to support i2c functions in Espruino,
 * a JavaScript interpreter for Microcontrollers designed by Gordon Williams
 *
 * Copyright (C) 2016 by Rhys Williams (wilberforce)
 * Modified Nov 2024 for ESP-IDF Version 5.2 by SimonGAndrews
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
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
#include "jshardwareI2c.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define TAG "jshardwareI2c"  // ESP-IDF log tag for debugging

// I2C acknowledgment settings
#define ACK_CHECK_EN  0x1   /*!< I2C master will check ACK from slave */
#define ACK_CHECK_DIS 0x0   /*!< I2C master will not check ACK from slave */
#define ACK_VAL       0x0   /*!< I2C ACK value */
#define NACK_VAL      0x1   /*!< I2C NACK value */

// I2C port and pin configuration, customizable as needed
#define I2C_MASTER_SCL_IO 19 /*!< GPIO number for I2C master clock */
#define I2C_MASTER_SDA_IO 18 /*!< GPIO number for I2C master data  */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

/**
 * Resets the I2C configuration to default.
 *
 * Relevant Example:
 * - I2C Master Example: https://github.com/espressif/esp-idf/tree/v5.2/examples/peripherals/i2c/i2c_master
 */
void I2CReset() {
    jsiConsolePrintf("jshardwareI2c.h - I2CReset: Resetting I2C\n");
    ESP_LOGI(TAG, "I2CReset: Resetting I2C to default configuration");

    // TODO: Implement reset logic to clear any custom configurations and return to default
}

/**
 * Initializes the I2C with specified device and settings.
 *
 * Relevant Example:
 * - I2C Master Example: https://github.com/espressif/esp-idf/tree/v5.2/examples/peripherals/i2c/i2c_master
 */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *info) {
    jsiConsolePrintf("jshardwareI2c.h - jshI2CSetup: Setting up I2C with device %d\n", device);
    ESP_LOGI(TAG, "jshI2CSetup: Configuring I2C for device %d", device);

    // TODO: Initialize I2C driver here with parameters from info
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    esp_err_t err = i2c_param_config(device, &i2c_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2C: %s", esp_err_to_name(err));
    }
}

/**
 * Writes data to the I2C bus.
 *
 * Relevant Example:
 * - I2C Master Write: https://github.com/espressif/esp-idf/tree/v5.2/examples/peripherals/i2c/i2c_master
 */
void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {
    jsiConsolePrintf("jshardwareI2c.h - jshI2CWrite: Writing %d bytes to device %d at address 0x%02X\n", nBytes, device, address);
    ESP_LOGI(TAG, "jshI2CWrite: Writing %d bytes to address 0x%02X on device %d", nBytes, address, device);

    // Example I2C write transaction (non-blocking mode can be implemented if needed)
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
} 

/** Read a number of bytes from the I2C device. */
void jshI2CRead(IOEventFlags device,
  unsigned char address,
  int nBytes,
  unsigned char *data,
  bool sendStop) {
  jsiConsolePrintf("jshardwareI2c.h - jshI2CRead");
}

#if original 
#include "jshardware.h"
#include "jshardwareI2c.h"
#include "driver/i2c.h"
#include "stdio.h"

#include "jsinteractive.h"

#define ACK_CHECK_EN   0x1   /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS  0x0   /*!< I2C master will not check ack from slave */

#define ACK_VAL  0x0     /*!< I2C ack value */
#define NACK_VAL   0x1     /*!< I2C nack value */

/* To do:
 support both i2c ports - done
 Test!
 Stop bits param -  bool sendStop
  https://esp-idf.readthedocs.io/en/latest/api/i2c.html

 */

void jshSetDeviceInitialised(IOEventFlags device, bool isInit);

static esp_err_t checkError( char * caller, esp_err_t ret ) {
  switch(ret) {
    case ESP_OK: break;
  case ESP_ERR_INVALID_ARG: {
    jsExceptionHere(JSET_ERROR, "%s:, Parameter error", caller );
    break;
  }
  case ESP_FAIL: {
    jsExceptionHere(JSET_ERROR, "%s: slave doesn't ACK the transfer", caller);
    break;
  }
  case ESP_ERR_TIMEOUT: {
    jsExceptionHere(JSET_ERROR, "%s:, Operation timeout because the bus is busy", caller );
    break;
  }
  default: {
    jsExceptionHere(JSET_ERROR, "%s:, unknown error code %d", caller, ret );
    break;
  }
  }
  return ret;
}

void I2CReset(){
  if(jshIsDeviceInitialised(EV_I2C1)){
    i2c_driver_delete(I2C_NUM_0);
    jshSetDeviceInitialised(EV_I2C1, false);
  }
#if ESPR_I2C_COUNT>1
  if(jshIsDeviceInitialised(EV_I2C2)){
    i2c_driver_delete(I2C_NUM_1);
    jshSetDeviceInitialised(EV_I2C2, false);
  }
#endif
}

int getI2cFromDevice( IOEventFlags device  ) {
  switch(device) {
  case EV_I2C1: return I2C_NUM_0;
#if ESPR_I2C_COUNT>1
  case EV_I2C2: return I2C_NUM_1;
#endif
  default: return -1;
  }
}

/** Set-up I2C master for ESP32, default pins are SCL:21, SDA:22. Only device I2C1 is supported
 *  and only master mode. */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *info) {
  int i2c_master_port = getI2cFromDevice(device);
  if (i2c_master_port == -1) {
    jsExceptionHere(JSET_ERROR,"Only I2C1 and I2C2 supported");
    return;
  }
  if(jshIsDeviceInitialised(device)){
  i2c_driver_delete(i2c_master_port);
  }
  Pin scl;
  Pin sda;
  if ( i2c_master_port == I2C_NUM_0 ) {
    scl = info->pinSCL != PIN_UNDEFINED ? info->pinSCL : 21;
    sda = info->pinSDA != PIN_UNDEFINED ? info->pinSDA : 22;
  }
#if ESPR_I2C_COUNT>1
  // Unsure on what to default these pins to?
  if ( i2c_master_port == I2C_NUM_1 ) {
    scl = info->pinSCL != PIN_UNDEFINED ? info->pinSCL : 16;
    sda = info->pinSDA != PIN_UNDEFINED ? info->pinSDA : 17;
  }
#endif

  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = pinToESP32Pin(sda);
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = pinToESP32Pin(scl);
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = info->bitrate;
  esp_err_t err=i2c_param_config(i2c_master_port, &conf);
  if ( err == ESP_ERR_INVALID_ARG ) {
    jsExceptionHere(JSET_ERROR,"jshI2CSetup: Invalid arguments");
  return;
  }
  err=i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
  if ( err == ESP_OK ) {
    jsDebug(DBG_INFO, "jshI2CSetup: driver installed, sda: %d scl: %d freq: %d, \n", sda, scl, info->bitrate);
    jshSetDeviceInitialised(device, true);
  } else {
    checkError("jshI2CSetup",err);
  }
}

void jshI2CWrite(IOEventFlags device,
  unsigned char address,
  int nBytes,
  const unsigned char *data,
  bool sendStop) {
  int i2c_master_port = getI2cFromDevice(device);
  if (i2c_master_port == -1) {
    jsExceptionHere(JSET_ERROR,"Only I2C1 and I2C2 supported");
    return;
  }
  esp_err_t ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  ret=i2c_master_start(cmd);
  ret=i2c_master_write_byte(cmd, address << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
  ret=i2c_master_write(cmd, data, nBytes, ACK_CHECK_EN);
  if ( sendStop ) ret=i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(i2c_master_port, cmd, 1000 / portTICK_RATE_MS); // 1000 seems very large for ticks_to_wait???
  i2c_cmd_link_delete(cmd);
  checkError(  "jshI2CWrite", ret);
}

void jshI2CRead(IOEventFlags device,
  unsigned char address,
  int nBytes,
  unsigned char *data,
  bool sendStop) {
    if (nBytes <= 0) {
    return;
  }
  int i2c_master_port = getI2cFromDevice(device);
  if (i2c_master_port == -1) {
    jsExceptionHere(JSET_ERROR,"Only I2C1 and I2C2 supported");
    return;
  }
  esp_err_t ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  ret=i2c_master_start(cmd);
  ret=i2c_master_write_byte(cmd, ( address << 1 ) | I2C_MASTER_READ, ACK_CHECK_EN);
  if (nBytes > 1) {
    ret=i2c_master_read(cmd, data, nBytes - 1, ACK_VAL);
  }
  ret=i2c_master_read_byte(cmd, data + nBytes - 1, NACK_VAL);
  if ( sendStop ) ret=i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(i2c_master_port, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  checkError(  "jshI2CRead", ret);
}
#endif // original 