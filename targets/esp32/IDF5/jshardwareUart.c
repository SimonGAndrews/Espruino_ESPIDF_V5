/*
 * This file is designed to support FREERTOS functions in Espruino,
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
 * Contains ESP32 board specific functions for UART setup and usage
 *  - See ESP-IDF v5 examples 
 *    https://github.com/espressif/esp-idf/tree/v5.2/examples/peripherals/uart/uart_echo
 * 
 * initUart();  Setup an ESP UARTS hardware 
 * UartReset();  Close down an ESP UART
 * 
 * initConsole(); Setup and map a UART as the Espruino console
 * 
 * initSerial(IOEventFlags device,JshUSARTInfo *inf); 
 * writeSerial(IOEventFlags device,uint8_t c);  
 * consoleToEspruino(); 
 * serialToEspruino(); 
 * ----------------------------------------------------------------------------
 */

#include "sdkconfig.h"  // Provides ESPIDF configs for default console settings
#include "driver/uart.h"
#include "esp_log.h"

#include "driver/usb_serial_jtag.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "jshardwareUart.h"
#include "jsinteractive.h"

bool serial2_initialized = false;
bool serial3_initialized = false;
extern void jshSetDeviceInitialised(IOEventFlags device, bool isInit);

// Define console UART settings with default values if not set by compiler
#ifndef CONSOLE_UART_NUM
  #define CONSOLE_UART_NUM UART_NUM_0
#endif

#ifndef CONSOLE_UART_BAUDRATE
  #define CONSOLE_UART_BAUDRATE 115200
#endif

#ifndef CONSOLE_UART_TX_PIN
  #define CONSOLE_UART_TX_PIN -1 // Use -1 for default TX pin
#endif

#ifndef CONSOLE_UART_RX_PIN
  #define CONSOLE_UART_RX_PIN -1 // Use -1 for default RX pin
#endif

// void initUart(int uart_num, uart_config_t uart_config, int txpin, int rxpin)
// Function to initialize a UART with specified configurations
void initUart(int uart_num, uart_config_t uart_config, int txpin, int rxpin) {
  esp_err_t err;

  // Set UART parameters
  err = uart_param_config(uart_num, &uart_config);
  if (err != ESP_OK) {
    ESP_LOGE("UART_INIT", "uart_param_config failed: %s", esp_err_to_name(err));
    return err;
  }

  // Set pins, using negative values to automatically select default pins if needed
  err = uart_set_pin(uart_num, txpin, rxpin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  if (err != ESP_OK) {
    ESP_LOGE("UART_INIT", "uart_set_pin failed: %s", esp_err_to_name(err));
    return err;
  }

  // (future use ?) use skdconfig CONFIG_UART_ISR_IN_IRAM to put the UART ISR function in IRAM 
  int intr_alloc_flags = 0;
#ifdef CONFIG_UART_ISR_IN_IRAM
  intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

  // Install UART driver 
  err = uart_driver_install(uart_num, 1024 * 2, 1024 * 2, 0, NULL, intr_alloc_flags);
  if (err != ESP_OK) {
    ESP_LOGE("UART_INIT", "uart_driver_install failed: %s", esp_err_to_name(err));
  }
}

// Function to initialize the console UART using defined configuration
// parameters
void initConsole() {
  int uart_num = CONSOLE_UART_NUM;
  int txpin = CONSOLE_UART_TX_PIN;
  int rxpin = CONSOLE_UART_RX_PIN;
  int baud_rate = CONSOLE_UART_BAUDRATE;

  uart_config_t uart_config = {.baud_rate = baud_rate,
                               .data_bits = UART_DATA_8_BITS,
                               .parity = UART_PARITY_DISABLE,
                               .stop_bits = UART_STOP_BITS_1,
                               .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                               .rx_flow_ctrl_thresh = 122};

  ESP_LOGI("INIT_CONSOLE",
           "Initializing console UART: UART%d, Baud Rate: %d, TX Pin: %d, RX "
           "Pin: %d", uart_num, baud_rate, txpin, rxpin);
  
  initUart(uart_num, uart_config, txpin, rxpin);

  // should we use hardware flow control on most ESP32 boards?
  // No... It looks like CTS is not connected on most boards, so XON/XOFF is
  // best!
  // jshSetFlowControlEnabled(EV_SERIAL1, true, PIN_UNDEFINED);
  // jshSetDeviceInitialised(EV_SERIAL1, true);
}

/**
 * Resets the UART configuration.
 */
void UartReset() {
  jsiConsolePrintf(
      "jshardwareUart.h - UartReset: Resetting UART configuration\n");
  ESP_LOGI("UartReset - ", "Resetting UART configuration");

  uart_driver_delete(uart_Serial1);
  initConsole();
  if (serial2_initialized)
    uart_driver_delete(uart_Serial2);
  if (serial3_initialized)
    uart_driver_delete(uart_Serial3);
}

void initSerial(IOEventFlags device, JshUSARTInfo *inf) {
  // NOTE: we can get called for bluetooth and telnet, so this may not be a
  // serial device!
  uart_config_t uart_config = {
      .baud_rate = inf->baudRate,
      .data_bits = (inf->bytesize == 7) ? UART_DATA_7_BITS : UART_DATA_8_BITS,
      .stop_bits = (inf->stopbits == 1) ? UART_STOP_BITS_1 : UART_STOP_BITS_2,
      //.flow_ctrl = (inf->xOnXOff)? UART_HW_FLOWCTRL_DISABLE : UART_HW_FLOWCTRL_CTS_RTS,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
      .parity = UART_PARITY_DISABLE};
  switch (inf->parity) {
  case 0:
    uart_config.parity = UART_PARITY_DISABLE;
    break;
  case 1:
    uart_config.parity = UART_PARITY_ODD;
    break;
  case 2:
    uart_config.parity = UART_PARITY_EVEN;
    break;
  }
  if (device == EV_SERIAL1) {
    initUart(uart_Serial1, uart_config, -1, -1);
    jshSetFlowControlEnabled(device, inf->xOnXOff, inf->pinCTS);
  } else if (device == EV_SERIAL2) {
    if (inf->pinTX == 0xff)
      inf->pinTX = 4;
    if (inf->pinRX == 0xff)
      inf->pinRX = 5;
    if (serial2_initialized)
      uart_driver_delete(uart_Serial2);
    initUart(uart_Serial2, uart_config, inf->pinTX, inf->pinRX);
    jshSetFlowControlEnabled(device, inf->xOnXOff, inf->pinCTS);
    jshSetDeviceInitialised(EV_SERIAL2, true);
    serial2_initialized = true;
#if ESPR_USART_COUNT > 2
  } else if (device == EV_SERIAL3) {
    if (inf->pinTX == 0xff)
      inf->pinTX = 17;
    if (inf->pinRX == 0xff)
      inf->pinRX = 16;
    if (serial3_initialized)
      uart_driver_delete(uart_Serial3);
    initUart(uart_Serial3, uart_config, inf->pinTX, inf->pinRX);
    jshSetFlowControlEnabled(device, inf->xOnXOff, inf->pinCTS);
    jshSetDeviceInitialised(EV_SERIAL3, true);
    serial3_initialized = true;
#endif
  }
}

/**
 * Writes a byte to the specified UART device.
 */
void writeSerial(IOEventFlags device, uint8_t c) {
  char str[2];
  int r;
  str[1] = '\0';
  str[0] = (char)c;
  if (device == EV_SERIAL2) {
    r = uart_write_bytes(uart_Serial2, (const char *)str, 1);
  } else {
    r = uart_write_bytes(uart_Serial3, (const char *)str, 1);
  }
}

uint8_t rxbuf[256];


void consoleToEspruino() {
  
int delay_ms = 10;  // delay 1000 ms 
//TickType_t ticksToWait = pdMS_TO_TICKS(delay_ms);  // Convert delay in milliseconds to ticks
uint32_t ticksToWait = pdMS_TO_TICKS(delay_ms);  // Convert delay in milliseconds to ticks

#ifdef CONFIG_IDF_TARGET_ESP32C3
#ifdef USB_CDC
  int len = usb_serial_jtag_read_bytes(rxbuf, sizeof(rxbuf), ticksToWait);
#else
  int len = uart_read_bytes(uart_Serial1, rxbuf, sizeof(rxbuf),
                            ticksToWait); // Read data from UART
#endif
#else
  int len = uart_read_bytes(uart_Serial1, rxbuf, sizeof(rxbuf),
                            ticksToWait); // Read data from UART
#endif
  if (len > 0){
    // jshPushIOCharEvents(EV_SERIAL1, rxbuf, len);
    uart_write_bytes(uart_Serial1, rxbuf, len);
  }
}

void serialToEspruino() {
  int len;
  if (serial2_initialized) {
    len = uart_read_bytes(uart_Serial2, rxbuf, sizeof(rxbuf), 0);
    if (len > 0)
      jshPushIOCharEvents(EV_SERIAL2, rxbuf, len);
  }
#if ESPR_USART_COUNT > 2
  if (serial3_initialized) {
    len = uart_read_bytes(uart_Serial3, rxbuf, sizeof(rxbuf), 0);
    if (len > 0)
      jshPushIOCharEvents(EV_SERIAL3, rxbuf, len);
  }
#endif
}

#if original
#include "driver/uart.h"
#include "jshardwareUart.h"

#include <jsdevices.h>
#include <stdio.h>
#include <string.h>

#ifdef CONFIG_IDF_TARGET_ESP32C3
#include "driver/usb_serial_jtag.h"
#endif

bool serial2_initialized = false;
bool serial3_initialized = false;

void jshSetDeviceInitialised(IOEventFlags device, bool isInit);

void initUart(int uart_num, uart_config_t uart_config, int txpin, int rxpin) {
  int r;
  r = uart_param_config(uart_num, &uart_config); // Configure UART1 parameters
  r = uart_set_pin(
      uart_num, txpin, rxpin, -1,
      -1); // Set UART0 pins(TX: IO16, RX: IO17, RTS: IO18, CTS: IO19)
  r = uart_driver_install(
      uart_num, 1024, 1024, 10, NULL,
      0); // Install UART driver( We don't need an event queue here)
}

void UartReset() {
  uart_driver_delete(uart_Serial1);
  initConsole();
  if (serial2_initialized)
    uart_driver_delete(uart_Serial2);
  if (serial3_initialized)
    uart_driver_delete(uart_Serial3);
}

void initSerial(IOEventFlags device, JshUSARTInfo *inf) {
  // NOTE: we can get called for bluetooth and telnet, so this may not be a
  // serial device!
  uart_config_t uart_config = {
      .baud_rate = inf->baudRate,
      .data_bits = (inf->bytesize == 7) ? UART_DATA_7_BITS : UART_DATA_8_BITS,
      .stop_bits = (inf->stopbits == 1) ? UART_STOP_BITS_1 : UART_STOP_BITS_2,
      //.flow_ctrl = (inf->xOnXOff)? UART_HW_FLOWCTRL_DISABLE :
      // UART_HW_FLOWCTRL_CTS_RTS,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
      .parity = UART_PARITY_DISABLE};
  switch (inf->parity) {
  case 0:
    uart_config.parity = UART_PARITY_DISABLE;
    break;
  case 1:
    uart_config.parity = UART_PARITY_ODD;
    break;
  case 2:
    uart_config.parity = UART_PARITY_EVEN;
    break;
  }
  if (device == EV_SERIAL1) {
    initUart(uart_Serial1, uart_config, -1, -1);
    jshSetFlowControlEnabled(device, inf->xOnXOff, inf->pinCTS);
  } else if (device == EV_SERIAL2) {
    if (inf->pinTX == 0xff)
      inf->pinTX = 4;
    if (inf->pinRX == 0xff)
      inf->pinRX = 5;
    if (serial2_initialized)
      uart_driver_delete(uart_Serial2);
    initUart(uart_Serial2, uart_config, inf->pinTX, inf->pinRX);
    jshSetFlowControlEnabled(device, inf->xOnXOff, inf->pinCTS);
    jshSetDeviceInitialised(EV_SERIAL2, true);
    serial2_initialized = true;
#if ESPR_USART_COUNT > 2
  } else if (device == EV_SERIAL3) {
    if (inf->pinTX == 0xff)
      inf->pinTX = 17;
    if (inf->pinRX == 0xff)
      inf->pinRX = 16;
    if (serial3_initialized)
      uart_driver_delete(uart_Serial3);
    initUart(uart_Serial3, uart_config, inf->pinTX, inf->pinRX);
    jshSetFlowControlEnabled(device, inf->xOnXOff, inf->pinCTS);
    jshSetDeviceInitialised(EV_SERIAL3, true);
    serial3_initialized = true;
#endif
  }
}

void initConsole() {
#ifdef CONFIG_IDF_TARGET_ESP32C3
#ifdef USB_CDC
  /* Configure USB-CDC */
  usb_serial_jtag_driver_config_t usb_serial_config = {.tx_buffer_size = 128,
                                                       .rx_buffer_size = 128};

  ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_config));
#endif
#endif

  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
  };
  initUart(uart_Serial1, uart_config, -1, -1);

  // should we use hardware flow control on most ESP32 boards?
  // No... It looks like CTS is not connected on most boards, so XON/XOFF is
  // best!
  jshSetFlowControlEnabled(EV_SERIAL1, true, PIN_UNDEFINED);
  jshSetDeviceInitialised(EV_SERIAL1, true);
}

uint8_t rxbuf[256];
void consoleToEspruino() {
  TickType_t ticksToWait = 100;
#if ESP_IDF_VERSION_MAJOR >= 4
  ticksToWait = 50 / portTICK_RATE_MS;
#endif
#ifdef CONFIG_IDF_TARGET_ESP32C3
#ifdef USB_CDC
  int len = usb_serial_jtag_read_bytes(rxbuf, sizeof(rxbuf), ticksToWait);
#else
  int len = uart_read_bytes(uart_Serial1, rxbuf, sizeof(rxbuf),
                            ticksToWait); // Read data from UART
#endif
#else
  int len = uart_read_bytes(uart_Serial1, rxbuf, sizeof(rxbuf),
                            ticksToWait); // Read data from UART
#endif
  if (len > 0)
    jshPushIOCharEvents(EV_SERIAL1, rxbuf, len);
}

void serialToEspruino() {
  int len;
  if (serial2_initialized) {
    len = uart_read_bytes(uart_Serial2, rxbuf, sizeof(rxbuf), 0);
    if (len > 0)
      jshPushIOCharEvents(EV_SERIAL2, rxbuf, len);
  }
#if ESPR_USART_COUNT > 2
  if (serial3_initialized) {
    len = uart_read_bytes(uart_Serial3, rxbuf, sizeof(rxbuf), 0);
    if (len > 0)
      jshPushIOCharEvents(EV_SERIAL3, rxbuf, len);
  }
#endif
}

void writeSerial(IOEventFlags device, uint8_t c) {
  char str[2];
  int r;
  str[1] = '\0';
  str[0] = (char)c;
  if (device == EV_SERIAL2) {
    r = uart_write_bytes(uart_Serial2, (const char *)str, 1);
  } else {
    r = uart_write_bytes(uart_Serial3, (const char *)str, 1);
  }
}
#endif // original