/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
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


/**
 * The ESP32 must implement its part of the Espruino contract.  This file
 * provides implementations for interfaces that are expected to be provided
 * by an Espruino board.  The signatures of the exposed functions are part
 * of the Espruino environment and can not be changed without express
 * approval from all the stakeholders.  In addition, the semantics of the
 * functions should follow the expected conventions.
 */

#include "jsinteractive.h"
#include "jshardware.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/uart.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "driver/timer.h"
#include "esp_log.h"

#define TAG "jshardware"
#define FLASH_MAX (4*1024*1024) //4MB
#define FLASH_PAGE_SHIFT 12 // Shift is much faster than division by 4096 (size of page)
#define FLASH_PAGE ((uint32_t)1<<FLASH_PAGE_SHIFT)  //4KB

#define UNUSED(x) (void)(x)


// System Initialization
void jshInit() {
    jsiConsolePrintf("jshardware.h - jshInit: Initializing hardware\n");
    ESP_LOGI(TAG, "Initializing hardware at startup");
}

void jshReset() {
    jsiConsolePrintf("jshardware.h - jshReset: Resetting hardware\n");
    ESP_LOGI(TAG, "Resetting peripherals to power-on state");
}

// Main Loop & Sleep Functions
void jshIdle() {
    jsiConsolePrintf("jshardware.h - jshIdle: Executing idle loop\n");
    ESP_LOGI(TAG, "Idle loop - checking for GPIO interrupts and updating state");
}

void jshBusyIdle() {
    jsiConsolePrintf("jshardware.h - jshBusyIdle: Busy idle\n");
    ESP_LOGI(TAG, "Handling busy-wait states (e.g., waiting for data to send)");
}

bool jshSleep(JsSysTime timeUntilWake) {
    jsiConsolePrintf("jshardware.h - jshSleep: Entering sleep mode\n");
    ESP_LOGI(TAG, "Entering sleep mode, wake after %llu ticks", timeUntilWake);
    return true;  // Stub return - implement sleep functionality
}

// Device Info & State
void jshKill() {
    jsiConsolePrintf("jshardware.h - jshKill: Cleaning up hardware\n");
    ESP_LOGI(TAG, "Cleaning up hardware resources");
}

int jshGetSerialNumber(unsigned char *data, int maxChars) {
    jsiConsolePrintf("jshardware.h - jshGetSerialNumber: Getting serial number\n");
    ESP_LOGI(TAG, "Getting hardware serial number");
    return 0; // Return actual number of chars
}

bool jshIsUSBSERIALConnected() {
    jsiConsolePrintf("jshardware.h - jshIsUSBSERIALConnected: Checking USB serial connection\n");
    ESP_LOGI(TAG, "Checking if USB serial is connected");
    return false; // Stub return value
}

JsSysTime jshGetSystemTime() {
    jsiConsolePrintf("jshardware.h - jshGetSystemTime: Getting system time\n");
    ESP_LOGI(TAG, "Retrieving system time since epoch");
    return 0;  // Stub return value
}

void jshSetSystemTime(JsSysTime time) {
    jsiConsolePrintf("jshardware.h - jshSetSystemTime: Setting system time\n");
    ESP_LOGI(TAG, "Setting system time to %llu", time);
}

/** Is the given device initialised?
 * eg. has jshUSARTSetup/jshI2CSetup/jshSPISetup been called previously? 
 * SGA TODO review this
 * */
bool jshIsDeviceInitialised(IOEventFlags device) {
  // uint64_t mask = 1ULL << (int)device;
  //return (DEVICE_INITIALISED_FLAGS & mask) != 0L;
  IOEventFlags dummy = 0; // SGA todo 
  return dummy;
} 

// GPIO Functions
void jshPinSetValue(Pin pin, bool value) {
    jsiConsolePrintf("jshardware.h - jshPinSetValue: Setting pin %d to %d\n", pin, value);
    ESP_LOGI(TAG, "Setting GPIO pin %d to %d", pin, value);
    gpio_set_level(pin, value);  // Actual GPIO output
}

bool jshPinGetValue(Pin pin) {
    jsiConsolePrintf("jshardware.h - jshPinGetValue: Reading pin %d\n", pin);
    ESP_LOGI(TAG, "Reading GPIO pin %d", pin);
    return gpio_get_level(pin); // Actual GPIO read
}

void jshPinSetState(Pin pin, JshPinState state) {
    jsiConsolePrintf("jshardware.h - jshPinSetState: Setting pin %d to state %d\n", pin, state);
    ESP_LOGI(TAG, "Setting pin %d to state %d", pin, state);
    // Configure GPIO pin state
}

JshPinState jshPinGetState(Pin pin) {
    jsiConsolePrintf("jshardware.h - jshPinGetState: Getting pin state for pin %d\n", pin);
    ESP_LOGI(TAG, "Retrieving pin state for pin %d", pin);
    JshPinState dummy = 0;
    return dummy; // Stub return SGA TODO
}

/** Given a pin function, set that pin to the 16 bit value
 * (used mainly for fast DAC and PWM handling from Utility Timer) */
void jshSetOutputValue(JshPinFunction func, int value) {
  int pin;
  if (JSH_PINFUNCTION_IS_DAC(func)) {
    uint8_t val = (uint8_t)(value >> 8);
    switch (func & JSH_MASK_INFO) {
  //    case JSH_DAC_CH1:  writeDAC(25,val); break;  SGA todo
  //    case JSH_DAC_CH2:  writeDAC(26,val); break;  SGA todo
    }
  }
  else{
    pin = ((func >> JSH_SHIFT_INFO) << 4) + ((func >> JSH_SHIFT_TYPE) & 15);
    // convert the 16 bit value to a 10 bit value.
    // value=value >> (16 - PWMTimerBit); SGA todo
    // setPWM( (Pin)pin, (uint16_t)value); SGA todo
  }
}

/**
 * Determine whether the pin can be watchable.
 * Returns true if the pin is watchable.
 * SGA TODO - review this
 */
bool jshCanWatch(
    Pin pin //!< The pin that we are asking whether or not we can watch it.
  ) {
#ifdef CONFIG_IDF_TARGET_ESP32C3
  return (pin!=18) && (pin!=19); // USB
#else
  return pin == 0 || ( pin >= 12 && pin <= 19 ) || pin == 21 ||  pin == 22 || ( pin >= 25 && pin <= 27 ) || ( pin >= 34 && pin <= 39 );
#endif
}

/// Given a Pin, return the current pin function associated with it
JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  if (jshIsPinValid(pin)) {
    int i;
    for (i=0;i<JSH_PININFO_FUNCTIONS;i++) {
      JshPinFunction func = pinInfo[pin].functions[i];
      if (JSH_PINFUNCTION_IS_TIMER(func) ||
          JSH_PINFUNCTION_IS_DAC(func))
        return func;
    }
  }
  return JSH_NOTHING;
}

/**
 * Check if state is default - return true if default
 * bool jshIsPinStateDefault(Pin pin, JshPinState state) 
 * see weak definition in src/jshardware_common.c
 * Called from jsinteractive.c jsiDumpHardwareInitialisation 
 */


// Timer Functions
void jshUtilTimerStart(JsSysTime period) {
    jsiConsolePrintf("jshardware.h - jshUtilTimerStart: Starting utility timer for %llu ticks\n", period);
    ESP_LOGI(TAG, "Starting utility timer with period %llu", period);
    // Start timer
}

void jshUtilTimerDisable() {
    jsiConsolePrintf("jshardware.h - jshUtilTimerDisable: Disabling utility timer\n");
    ESP_LOGI(TAG, "Disabling utility timer");
    // Stop timer
}

// USART, I2C, and Flash Stubs
void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
    jsiConsolePrintf("jshardware.h - jshUSARTSetup: Setting up UART device %d\n", device);
    ESP_LOGI(TAG, "Setting up UART device %d with specified info", device);
}

void jshFlashWrite(void *buf, uint32_t addr, uint32_t len) {
    jsiConsolePrintf("jshardware.h - jshFlashWrite: Writing to flash at address %08x, length %d\n", addr, len);
    ESP_LOGI(TAG, "Writing to flash memory at address %08x, length %d", addr, len);
}

/** Kick a device into action (if required). For instance we may have data ready
 * to sent to a USART, but we need to enable the IRQ such that it can automatically
 * fetch the characters to send.
 */
void jshUSARTKick(IOEventFlags device) {
  int c = jshGetCharToTransmit(device);
  while(c >= 0) {
  switch(device){
#ifdef BLUETOOTH
    case EV_BLUETOOTH:
      gatts_sendNUSNotification(c);
      break;
#endif
    case EV_SERIAL1:
 //     uart_tx_one_char((uint8_t)c); //SGA todo
#ifdef CONFIG_IDF_TARGET_ESP32C3
      // The USB CDC UART on the C3 only writes the data to USB after a newline. Ensure uartTask in main.c knows to flush the UART next time
      extern void esp32USBUARTWasUsed();
      esp32USBUARTWasUsed();
#endif
      break;
    default:
     // writeSerial(device,(uint8_t)c); SGA todo
      break;
    //if(device == EV_SERIAL1) uart_tx_one_char((uint8_t)c);
    //else writeSerial(device,(uint8_t)c);
  }
    c = jshGetCharToTransmit(device);
  }
}


// Analog Functions
JsVarFloat jshPinAnalog(Pin pin) {
    jsiConsolePrintf("jshardware.h - jshPinAnalog: Reading analog value on pin %d\n", pin);
    ESP_LOGI(TAG, "Reading analog value on pin %d", pin);
    return 0.0; // Stub return
}

int jshPinAnalogFast(Pin pin) {
    jsiConsolePrintf("jshardware.h - jshPinAnalogFast: Fast analog read on pin %d\n", pin);
    ESP_LOGI(TAG, "Fast analog read on pin %d", pin);
    return 0; // Stub return
}

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) {
    jsiConsolePrintf("jshardware.h - jshPinAnalogOutput: Analog output on pin %d, value=%f, freq=%f\n", pin, value, freq);
    ESP_LOGI(TAG, "Setting analog output on pin %d with value %f and frequency %f", pin, value, freq);
    JshPinFunction dummy = 0;
    return dummy; // Stub return  SGA TODO
}

// Watchdog Functions
void jshEnableWatchDog(JsVarFloat timeout) {
    jsiConsolePrintf("jshardware.h - jshEnableWatchDog: Enabling watchdog with timeout %f seconds\n", timeout);
    ESP_LOGI(TAG, "Enabling watchdog with timeout %f seconds", timeout);
}

void jshKickWatchDog() {
    jsiConsolePrintf("jshardware.h - jshKickWatchDog: Kicking watchdog\n");
    ESP_LOGI(TAG, "Kicking watchdog to prevent reset");
}

/* not needed here its in jshardware_common 
void jshKickSoftWatchDog() {
    jsiConsolePrintf("jshardware.h - jshKickSoftWatchDog: Kicking soft watchdog\n");
    ESP_LOGI(TAG, "Kicking soft watchdog");
}
*/

// Event and Pin Watch Functions
IOEventFlags jshPinWatch(Pin pin, bool shouldWatch, JshPinWatchFlags flags) {
    jsiConsolePrintf("jshardware.h - jshPinWatch: Watching pin %d, shouldWatch=%d\n", pin, shouldWatch);
    ESP_LOGI(TAG, "Setting watch on pin %d, shouldWatch=%d", pin, shouldWatch);
    IOEventFlags dummy=0;
    return dummy; // Stub return SGA TODO
}

bool jshGetWatchedPinState(IOEventFlags device) {
    jsiConsolePrintf("jshardware.h - jshGetWatchedPinState: Checking watched pin state\n");
    ESP_LOGI(TAG, "Checking state of watched pin for device %d", device);
    return false; // Stub return
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
    jsiConsolePrintf("jshardware.h - jshIsEventForPin: Checking event for pin %d\n", pin);
    ESP_LOGI(TAG, "Checking if event is for pin %d", pin);
    return false; // Stub return
}

/**
 * Erase the flash page containing the address.
 * SGA todo review
 */
void jshFlashErasePage(
    uint32_t addr //!<
  ) {
#if ESP_IDF_VERSION_MAJOR>=5
 // esp_flash_erase_region(NULL, addr >> FLASH_PAGE_SHIFT, FLASH_PAGE); SGA TOIDI
#else
 // spi_flash_erase_sector(addr >> FLASH_PAGE_SHIFT);
#endif
}

void jshFlashRead(void *buf, uint32_t addr, uint32_t len) {
    jsiConsolePrintf("jshardware.h - jshFlashRead: Reading flash at addr %08x, length %d\n", addr, len);
    ESP_LOGI(TAG, "Reading flash memory at addr %08x, length %d", addr, len);
    // Stub read
}

size_t jshFlashGetMemMapAddress(size_t ptr) {
    jsiConsolePrintf("jshardware.h - jshFlashGetMemMapAddress: Mapping memory address %08x\n", ptr);
    ESP_LOGI(TAG, "Mapping memory address %08x", ptr);
    return ptr; // Stub return
}


/*
getFree function called from jswrap_flash_getFree
This method returns an array of objects of the form `{addr : #, length : #}`,
representing contiguous areas of flash memory in the chip that are not used for
anything.
*/
JsVar *jshFlashGetFree() {
  JsVar *jsFreeFlash = jsvNewEmptyArray();
  if (!jsFreeFlash) return 0;
  // Space reserved here in the parition table -  using sub type 0x40
  // This should be read from the partition table  todo ?????
 // addFlashArea(jsFreeFlash, 0xE000, 0x2000);   SGA TODO
 // addFlashArea(jsFreeFlash, 0x310000, 0x10000);
 // addFlashArea(jsFreeFlash, 0x360000, 0xA0000);
  return jsFreeFlash;
}

/**
 * Return start address and size of the flash page the given address resides in.
 * Returns false if no page.
 */
bool jshFlashGetPage(
    uint32_t addr,       //!<
    uint32_t *startAddr, //!<
    uint32_t *pageSize   //!<
  ) {
  if (addr >= FLASH_MAX) return false;
  *startAddr = addr & ~(FLASH_PAGE-1);
  *pageSize = FLASH_PAGE;
  return true;
}

// Utility Timer Control
void jshUtilTimerReschedule(JsSysTime period) {
    jsiConsolePrintf("jshardware.h - jshUtilTimerReschedule: Rescheduling timer for %llu ticks\n", period);
    ESP_LOGI(TAG, "Rescheduling timer for period %llu", period);
    // Stub reschedule timer
}

// Miscellaneous Functions
JsVarFloat jshReadTemperature() {
    jsiConsolePrintf("jshardware.h - jshReadTemperature: Reading temperature\n");
    ESP_LOGI(TAG, "Reading temperature from internal sensor");
    return 0.0; // Stub temperature value
}

JsVarFloat jshReadVRef() {
    jsiConsolePrintf("jshardware.h - jshReadVRef: Reading voltage reference\n");
    ESP_LOGI(TAG, "Reading voltage reference");
    return 3.3; // Stub voltage reference (example 3.3V)
}

unsigned int jshGetRandomNumber() {
    jsiConsolePrintf("jshardware.h - jshGetRandomNumber: Generating random number\n");
    ESP_LOGI(TAG, "Generating random number");
    return rand(); // Stub using rand()
}

void jshReboot() {
    jsiConsolePrintf("jshardware.h - jshReboot: Rebooting device\n");
    ESP_LOGI(TAG, "Rebooting device");
    // Stub for system reboot
}

/** Change the processor clock info. What's in options is platform
 * specific - you should update the docs for jswrap_espruino_setClock
 * to match what gets implemented here. The return value is the clock
 * speed in Hz though. */
unsigned int jshSetSystemClock(JsVar *options) {
  UNUSED(options);
  jsError(">> jshSetSystemClock Not implemented");
  return 0;
}

/* Adds the estimated power usage of the microcontroller in uA to the 'devices' object. The CPU should be called 'CPU' */
// also defined with a weak function in jshardware_common.c where // not implemented by default
void jsvGetProcessorPowerUsage(JsVar *devices) {
  jsvObjectSetChildAndUnLock(devices, "CPU", jsvNewFromInteger(20000));
  // standard power usage of ESP32S3 without Wifi
}


/** Get processor clock info. What's returned is platform
 * specific - you should update the docs for jswrap_espruino_getClock
 * to match what gets implemented here */
// JsVar *jshGetSystemClock(); is defined in jshardware_common with a weak definition

/**
 * Given a time in microseconds, get us the value in milliseconds (float)
 */
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return (JsVarFloat) time / 1000.0;
}

void jshInterruptOff() {
  //taskDISABLE_INTERRUPTS();
}

void jshInterruptOn()  {
  //taskENABLE_INTERRUPTS();
}

/// Are we currently in an interrupt?
bool jshIsInInterrupt() {
  return false; // FIXME!
}

/**
 * Delay (blocking) for the supplied number of microseconds.
 */
void jshDelayMicroseconds(int microsec) {
  // ets_delay_us((uint32_t)microsec);
} // End of jshDelayMicroseconds

/**
 * Given a time in milliseconds as float, get us the value in microsecond
 */
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime) (ms * 1000.0);
}

#if original
#include <stdio.h>
#include <sys/time.h>

#include "jshardware.h"
#include "jshardwareUart.h"
#include "jshardwareAnalog.h"
#include "jshardwarePWM.h"
#include "jshardwarePulse.h"
#include "rtosutil.h"
#include "driver/timer.h"

#ifdef BLUETOOTH
#include "BLE/esp32_gap_func.h"
#include "BLE/esp32_gattc_func.h"
#include "BLE/esp32_gatts_func.h"
#endif
#include "jshardwareESP32.h"

#include "jsutils.h"
#include "jstimer.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jspininfo.h"

#include "jswrap_esp32_network.h"

#if ESP_IDF_VERSION_MAJOR>=4
#include "soc/uart_reg.h"
#include "esp_mac.h"
#endif
#include "esp_attr.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "rom/ets_sys.h"
#include "rom/uart.h"
#include "driver/gpio.h"
#include "soc/gpio_sig_map.h"

#if ESP_IDF_VERSION_MAJOR>=5
#include "esp_flash.h"
#include "soc/gpio_reg.h"
#else
#include "esp_spi_flash.h"
#endif

#include "jshardwareI2c.h"
#include "jshardwareSpi.h"

#define FLASH_MAX (4*1024*1024) //4MB
#define FLASH_PAGE_SHIFT 12 // Shift is much faster than division by 4096 (size of page)
#define FLASH_PAGE ((uint32_t)1<<FLASH_PAGE_SHIFT)  //4KB

#define UNUSED(x) (void)(x)

/**
 * Convert a pin id to the corresponding Pin Event id.
 */
static IOEventFlags pinToEV_EXTI(
    Pin pin // !< The pin to map to the event id.
  ) {
  // Map pin 0 to EV_EXTI0
  // Map pin 1 to EV_EXTI1
  // ...
  // Map pin x to ECEXTIx
  return (IOEventFlags)(EV_EXTI0 + pin);
}

static uint8_t g_pinState[JSH_PIN_COUNT];

/// Whether a pin is being used for soft PWM or not
BITFIELD_DECL(jshPinSoftPWM, JSH_PIN_COUNT);

/// Has the watchdog been enabled?
bool wdt_enabled = false;

static uint64_t DEVICE_INITIALISED_FLAGS = 0L;

void jshSetDeviceInitialised(IOEventFlags device, bool isInit) {
  uint64_t mask = 1ULL << (int)device;
  if (isInit) {
    DEVICE_INITIALISED_FLAGS |= mask;
  } else {
    DEVICE_INITIALISED_FLAGS &= ~mask;
  }
}

/**
* interrupt handler for gpio interrupts
*/
void IRAM_ATTR gpio_intr_handler(void* arg){
  //GPIO intr process. Mainly copied from esp-idf
  UNUSED(arg);
  IOEventFlags exti;
  Pin gpio_num = 0;
  uint32_t gpio_intr_status = READ_PERI_REG(GPIO_STATUS_REG);   //read status to get interrupt status for GPIO0-31
#ifndef CONFIG_IDF_TARGET_ESP32C3
  uint32_t gpio_intr_status_h = READ_PERI_REG(GPIO_STATUS1_REG);//read status1 to get interrupt status for GPIO32-39
#endif
  SET_PERI_REG_MASK(GPIO_STATUS_W1TC_REG, gpio_intr_status);    //Clear intr for gpio0-gpio31
#ifndef CONFIG_IDF_TARGET_ESP32C3
  SET_PERI_REG_MASK(GPIO_STATUS1_W1TC_REG, gpio_intr_status_h); //Clear intr for gpio32-39
#endif

  do {
    if(gpio_num < 32) {
      if(gpio_intr_status & BIT(gpio_num)) { //gpio0-gpio31
         exti = pinToEV_EXTI(gpio_num);
         jshPushIOWatchEvent(exti);
      }
    } else {
#ifndef CONFIG_IDF_TARGET_ESP32C3
      if(gpio_intr_status_h & BIT(gpio_num - 32)) {
        exti = pinToEV_EXTI(gpio_num);
        jshPushIOWatchEvent(exti);
      }
#endif
    }
  } while(++gpio_num < JSH_PIN_COUNT);
}

void jshPinSetStateRange( Pin start, Pin end, JshPinState state ) {
    for ( Pin p=start; p<=end; p++ ) {
        jshPinSetState(p, state);
    }
}

void jshPinDefaultPullup() {
#ifdef CONFIG_IDF_TARGET_ESP32C3

#else
  // 6-11 are used by Flash chip
  // 32-33 are routed to rtc for xtal
  // 16-17 are used for PSRAM (future use)
  jshPinSetStateRange(0,0,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(12,15,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(18,19,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(21,22,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(25,27,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(34,39,JSHPINSTATE_GPIO_IN_PULLUP);
#endif
}

/**
 * Initialize the JavaScript hardware interface.
 */
void jshInit() {
  if(ESP32_Get_NVS_Status(ESP_NETWORK_WIFI)) esp32_wifi_init();
#ifdef BLUETOOTH
  if(ESP32_Get_NVS_Status(ESP_NETWORK_BLE)) gattc_init();
#endif
  jshInitDevices();
  BITFIELD_CLEAR(jshPinSoftPWM);
  if (JSHPINSTATE_I2C != 13 || JSHPINSTATE_GPIO_IN_PULLDOWN != 6 || JSHPINSTATE_MASK != 15) {
    jsError("JshPinState #defines have changed, please update pinStateToString()");
  }
  gpio_isr_register(gpio_intr_handler,NULL,0,NULL);  //changed to automatic assign of interrupt
  // Initialize something for each of the possible pins.
  jshPinDefaultPullup();
} // End of jshInit

void jshKill() {
}

/**
 * Reset the Espruino environment.
 */
void jshReset() {
  jshResetDevices();
  jshPinDefaultPullup() ;
//  UartReset();
  RMTReset();
  ADCReset();
  SPIReset();
  I2CReset();
#ifdef BLUETOOTH
  if(ESP32_Get_NVS_Status(ESP_NETWORK_BLE)) gatts_reset(false);
#endif
}

/**
 * Re-init the ESP32 after a soft-reset
 */
void jshSoftInit() {
  if(ESP32_Get_NVS_Status(ESP_NETWORK_WIFI)) jswrap_esp32_wifi_soft_init();
}

/**
 * Handle whatever needs to be done in the idle loop when there's nothing to do.
 *
 * Nothing is needed on the ESP32.
 */
void jshIdle() {

}

// ESP32 chips don't have a serial number but they do have a MAC address
int jshGetSerialNumber(unsigned char *data, int maxChars) {
  assert(maxChars >= 6); // it's 32
  esp_efuse_mac_get_default(data);
  return 6;
}

void jshInterruptOff() {
  //taskDISABLE_INTERRUPTS();
}

void jshInterruptOn()  {
  //taskENABLE_INTERRUPTS();
}

/// Are we currently in an interrupt?
bool jshIsInInterrupt() {
  return false; // FIXME!
}

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
#if ESP_IDF_VERSION_MAJOR>=4
  double ms = jshGetMillisecondsFromTime(timeUntilWake);
  if (ms>50) ms=50; // hack for now - ideally jshHadEvent called from UART IRQs would break out of vTaskDelay
  vTaskDelay(ms / portTICK_PERIOD_MS);
#else
  UNUSED(timeUntilWake);
  // we never sleep in older IDFs
#endif
  return true;
} // End of jshSleep


/**
 * Delay (blocking) for the supplied number of microseconds.
 */
void jshDelayMicroseconds(int microsec) {
  ets_delay_us((uint32_t)microsec);
} // End of jshDelayMicroseconds



/**
 * Set the state of the specific pin.
 *
 * The possible states are:
 *
 * JSHPINSTATE_UNDEFINED
 * JSHPINSTATE_GPIO_OUT
 * JSHPINSTATE_GPIO_OUT_OPENDRAIN
 * JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP
 * JSHPINSTATE_GPIO_IN
 * JSHPINSTATE_GPIO_IN_PULLUP
 * JSHPINSTATE_GPIO_IN_PULLDOWN
 * JSHPINSTATE_ADC_IN
 * JSHPINSTATE_AF_OUT
 * JSHPINSTATE_AF_OUT_OPENDRAIN
 * JSHPINSTATE_USART_IN
 * JSHPINSTATE_USART_OUT
 * JSHPINSTATE_DAC_OUT
 * JSHPINSTATE_I2C
 *
 * This function is exposed indirectly through the exposed global function called
 * `pinMode()`.  For example, `pinMode(pin, "input")` will set the given pin to input.
 */
void jshPinSetState(
  Pin pin,                 //!< The pin to have its state changed.
    JshPinState state        //!< The new desired state of the pin.
  ) {
  /* Make sure we kill software PWM if we set the pin state
   * after we've started it */
  if (BITFIELD_GET(jshPinSoftPWM, pin)) {
    BITFIELD_SET(jshPinSoftPWM, pin, 0);
    jstPinPWM(0,0,pin);
  }
  gpio_mode_t mode;
  gpio_pull_mode_t pull_mode=GPIO_FLOATING;
  switch(state) {
  case JSHPINSTATE_GPIO_OUT:
    mode = GPIO_MODE_INPUT_OUTPUT;
    break;
  case JSHPINSTATE_GPIO_IN:
    mode = GPIO_MODE_INPUT;
    break;
  case JSHPINSTATE_GPIO_IN_PULLUP:
    mode = GPIO_MODE_INPUT;
    pull_mode=GPIO_PULLUP_ONLY;
    break;
  case JSHPINSTATE_GPIO_IN_PULLDOWN:
    mode = GPIO_MODE_INPUT;
    pull_mode=GPIO_PULLDOWN_ONLY;
    break;
  case JSHPINSTATE_GPIO_OUT_OPENDRAIN:
    mode = GPIO_MODE_INPUT_OUTPUT_OD;
    break;
  case JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP:
    mode = GPIO_MODE_INPUT_OUTPUT_OD;
    pull_mode=GPIO_PULLUP_ONLY;
    break;
  default:
    jsError( "jshPinSetState: Unexpected state: %d", state);
  return;
  }
  gpio_num_t gpioNum = pinToESP32Pin(pin);
  gpio_set_direction(gpioNum, mode);
  gpio_set_pull_mode(gpioNum, pull_mode);
#if ESP_IDF_VERSION_MAJOR>=5
  esp_rom_gpio_pad_select_gpio(gpioNum);
#else
  gpio_pad_select_gpio(gpioNum);
#endif
  g_pinState[pin] = state; // remember what we set this to...
}


/**
 * Return the current state of the selected pin.
 * \return The current state of the selected pin.
 */
JshPinState jshPinGetState(Pin pin) {
  if ( jshPinGetValue(pin) & 1 )
    return g_pinState[pin] | JSHPINSTATE_PIN_IS_ON;
  return g_pinState[pin];
}

/**
 * Check if state is default - return true if default
*/
bool jshIsPinStateDefault(Pin pin, JshPinState state) {
  return state == JSHPINSTATE_GPIO_IN_PULLUP || state == JSHPINSTATE_ADC_IN;
}

//===== GPIO and PIN stuff =====

/**
 * Set the value of the corresponding pin.
 */
void jshPinSetValue(
    Pin pin,   //!< The pin to have its value changed.
    bool value //!< The new value of the pin.
  ) {
  gpio_num_t gpioNum = pinToESP32Pin(pin);
#if ESP_IDF_VERSION_MAJOR>=5
  gpio_iomux_out(gpioNum,SIG_GPIO_OUT_IDX,0);  // reset pin to be GPIO in case it was used as rmt or something else
#else
  gpio_matrix_out(gpioNum,SIG_GPIO_OUT_IDX,0,0);  // reset pin to be GPIO in case it was used as rmt or something else
#endif
  gpio_set_level(gpioNum, (uint32_t)value);
}


/**
 * Get the value of the corresponding pin.
 * \return The current value of the pin.
 */
bool CALLED_FROM_INTERRUPT jshPinGetValue( // can be called at interrupt time
    Pin pin //!< The pin to have its value read.
  ) {
  gpio_num_t gpioNum = pinToESP32Pin(pin);
  bool level = gpio_get_level(gpioNum);
  return level;
}


JsVarFloat jshPinAnalog(Pin pin) {
  if (pinInfo[pin].analog == JSH_ANALOG_NONE)
    return NAN;
  return (JsVarFloat) readADC(pin) / 4096;
}


int jshPinAnalogFast(Pin pin) {
  if (pinInfo[pin].analog == JSH_ANALOG_NONE)
    return 0;
  return readADC(pin) << 4;
}


/**
 * Set the output PWM value.
 */
JshPinFunction jshPinAnalogOutput(Pin pin,
    JsVarFloat value,
    JsVarFloat freq,
    JshAnalogOutputFlags flags) { // if freq<=0, the default is used
  UNUSED(flags);
  if (value<0) value=0;
  if (value>1) value=1;
  if (!isfinite(freq)) freq=0;
  if(pin == 25 || pin == 26){
  if(flags & (JSAOF_ALLOW_SOFTWARE | JSAOF_FORCE_SOFTWARE)) jsError("pin does not support software PWM");
    writeDAC(pin,(uint8_t)(value * 256));
  }
  else{
  if(flags & JSAOF_ALLOW_SOFTWARE){
    if (!jshGetPinStateIsManual(pin)){
        BITFIELD_SET(jshPinSoftPWM, pin, 0);
        jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
      }
      BITFIELD_SET(jshPinSoftPWM, pin, 1);
      if ((freq<=0)) freq=50;
      jstPinPWM(freq, value, pin);
      return 0;
    }
    else writePWM(pin,( uint16_t)(value * PWMTimerRange),(int) freq);
  }
  return 0;
}


/**
 *
 */
void jshSetOutputValue(JshPinFunction func, int value) {
  int pin;
  if (JSH_PINFUNCTION_IS_DAC(func)) {
    uint8_t val = (uint8_t)(value >> 8);
    switch (func & JSH_MASK_INFO) {
      case JSH_DAC_CH1:  writeDAC(25,val); break;
      case JSH_DAC_CH2:  writeDAC(26,val); break;
    }
  }
  else{
    pin = ((func >> JSH_SHIFT_INFO) << 4) + ((func >> JSH_SHIFT_TYPE) & 15);
    // convert the 16 bit value to a 10 bit value.
    value=value >> (16 - PWMTimerBit);
    setPWM( (Pin)pin, (uint16_t)value);
  }
}

void jshEnableWatchDog(JsVarFloat timeout) {
  wdt_enabled = true;
  esp_task_wdt_init((int)(timeout+0.5)
#if !(ESP_IDF_VERSION_MAJOR>=5)
   , true
#endif
  ); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
}

// Kick the watchdog
void jshKickWatchDog() {
  if (wdt_enabled)
    esp_task_wdt_reset();
}


/**
 * Get the state of the pin associated with the event flag.
 */
bool CALLED_FROM_INTERRUPT jshGetWatchedPinState(IOEventFlags eventFlag) { // can be called at interrupt time
  gpio_num_t gpioNum = pinToESP32Pin((Pin)(eventFlag-EV_EXTI0));
  bool level = gpio_get_level(gpioNum);
  return level;
}


/**
 * Determine whether the pin can be watchable.
 * \return Returns true if the pin is watchable.
 */
bool jshCanWatch(
    Pin pin //!< The pin that we are asking whether or not we can watch it.
  ) {
#ifdef CONFIG_IDF_TARGET_ESP32C3
  return (pin!=18) && (pin!=19); // USB
#else
  return pin == 0 || ( pin >= 12 && pin <= 19 ) || pin == 21 ||  pin == 22 || ( pin >= 25 && pin <= 27 ) || ( pin >= 34 && pin <= 39 );
#endif
}


/**
 * Do what ever is necessary to watch a pin.
 * \return The event flag for this pin.
 */
IOEventFlags jshPinWatch(
      Pin pin,          //!< The pin to be watched.
      bool shouldWatch, //!< True for watching and false for unwatching.
      JshPinWatchFlags flags
    ) {
  gpio_num_t gpioNum = pinToESP32Pin(pin);
  if(shouldWatch){
    gpio_set_intr_type(gpioNum,GPIO_INTR_ANYEDGE);             //set posedge interrupt
    gpio_set_direction(gpioNum,GPIO_MODE_INPUT);               //set as input
    gpio_set_pull_mode(gpioNum,GPIO_PULLUP_ONLY);              //enable pull-up mode
    gpio_intr_enable(gpioNum);                                 //enable interrupt
    return pinToEV_EXTI(gpioNum);
  } else{
    if(gpio_intr_disable(gpioNum) == ESP_ERR_INVALID_ARG){     //disable interrupt
        jsError("*** jshPinWatch error");
    }
  }
  return EV_NONE;
}


/**
 *
 */
JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  if (jshIsPinValid(pin)) {
    int i;
    for (i=0;i<JSH_PININFO_FUNCTIONS;i++) {
      JshPinFunction func = pinInfo[pin].functions[i];
      if (JSH_PINFUNCTION_IS_TIMER(func) ||
          JSH_PINFUNCTION_IS_DAC(func))
        return func;
    }
  }
  return JSH_NOTHING;
}


/**
 * Determine if a given event is associated with a given pin.
 * \return True if the event is associated with the pin and false otherwise.
 */
bool jshIsEventForPin(
    IOEvent *event, //!< The event that has been detected.
    Pin pin         //!< The identity of a pin.
  ) {
  return IOEVENTFLAGS_GETTYPE(event->flags) == pinToEV_EXTI(pin);
}

//===== USART and Serial =====



void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {

  if (inf->errorHandling) {
    jsExceptionHere(JSET_ERROR, "ESP32 Espruino builds can't handle framing/parity errors (errors:true)");
    return;
  }

  initSerial(device,inf);
}

bool jshIsUSBSERIALConnected() {
  return false; // "On non-USB boards this just returns false"
}

/**
 * Kick a device into action (if required).
 *
 */
void jshUSARTKick(IOEventFlags device) {
  int c = jshGetCharToTransmit(device);
  while(c >= 0) {
  switch(device){
#ifdef BLUETOOTH
    case EV_BLUETOOTH:
      gatts_sendNUSNotification(c);
      break;
#endif
    case EV_SERIAL1:
      uart_tx_one_char((uint8_t)c);
#ifdef CONFIG_IDF_TARGET_ESP32C3
      // The USB CDC UART on the C3 only writes the data to USB after a newline. Ensure uartTask in main.c knows to flush the UART next time
      extern void esp32USBUARTWasUsed();
      esp32USBUARTWasUsed();
#endif
      break;
    default:
      writeSerial(device,(uint8_t)c);
      break;
    //if(device == EV_SERIAL1) uart_tx_one_char((uint8_t)c);
    //else writeSerial(device,(uint8_t)c);
  }
    c = jshGetCharToTransmit(device);
  }
}

//===== System time stuff =====

/**
 * Given a time in milliseconds as float, get us the value in microsecond
 */
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime) (ms * 1000.0);
}

/**
 * Given a time in microseconds, get us the value in milliseconds (float)
 */
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return (JsVarFloat) time / 1000.0;
}


/**
 * Return the current time in microseconds.
 */
static portMUX_TYPE JSmicrosMux = portMUX_INITIALIZER_UNLOCKED;
JsSysTime CALLED_FROM_INTERRUPT jshGetSystemTime() { // in us -- can be called at interrupt time
  struct timeval tm;
  portENTER_CRITICAL_ISR(&JSmicrosMux);
  gettimeofday(&tm, 0);
  portEXIT_CRITICAL_ISR(&JSmicrosMux);
  return (JsSysTime)(tm.tv_sec)*1000000L + tm.tv_usec;
}

/**
 * Set the current time in microseconds.
 */
void jshSetSystemTime(JsSysTime newTime) {
  struct timeval tm;
  struct timezone tz;

  tm.tv_sec=(time_t)(newTime/1000000L);
  tm.tv_usec=(suseconds_t) (newTime - tm.tv_sec * 1000000L);
  tz.tz_minuteswest=0;
  tz.tz_dsttime=0;
  settimeofday(&tm, &tz);
}

void jshUtilTimerDisable() {
  timer_pause(TIMER_GROUP_0, 0);
  timer_disable_intr(TIMER_GROUP_0, 0);
}

void jshUtilTimerStart(JsSysTime period) {
  if(period <= 30){period = 30;}
  timer_Start(0, period);
}

void jshUtilTimerReschedule(JsSysTime period) {
  if(period <= 30){period = 30;}
  timer_Reschedule(0,(uint64_t)period);
}

//===== Miscellaneous =====

bool jshIsDeviceInitialised(IOEventFlags device) {
  uint64_t mask = 1ULL << (int)device;
  return (DEVICE_INITIALISED_FLAGS & mask) != 0L;

//  UNUSED(device);
//  jsError(">> jshIsDeviceInitialised not implemented");
// return 0;
} // End of jshIsDeviceInitialised


// the esp32 temperature sensor - undocumented library function call. Unsure of values returned.
JsVarFloat jshReadTemperature() {
  jsError(">> jshReadTemperature Not implemented");
  return NAN;
}

// the esp8266 can read the VRef but then there's no analog input, so we don't support this
JsVarFloat jshReadVRef() {
  jsError(">> jshReadVRef Not implemented");
  return NAN;
}

unsigned int jshGetRandomNumber() {
  return (unsigned int)rand();
}

//===== Read-write flash =====

/**
 * Determine available flash depending on EEprom size
 *
 */
uint32_t jshFlashMax() {
  return (FLASH_MAX-1);
}

/**
 * Read data from flash memory into the buffer.
 *
 * This reads from flash using memory-mapped reads. Only works for the first 1MB and
 * requires 4-byte aligned reads.
 *
 */
void jshFlashRead(
    void *buf,     //!< buffer to read into
    uint32_t addr, //!< Flash address to read from
    uint32_t len   //!< Length of data to read
  ) {

  if(len == 1){ // Can't read a single byte using the API, so read 4 and select the byte requested
    uint word;
#if ESP_IDF_VERSION_MAJOR>=5
    esp_flash_read(NULL, addr & 0xfffffffc,&word,4);
#else
    spi_flash_read(addr & 0xfffffffc,&word,4);
#endif
    *(uint8_t *)buf = (word >> ((addr & 3) << 3 )) & 255;
  } else {
#if ESP_IDF_VERSION_MAJOR>=5
    esp_flash_read(NULL, addr, buf, len);
#else
    spi_flash_read(addr, buf, len);
#endif
  }
}


/**
 * Write data to flash memory from the buffer.
 *
 * This is called from jswrap_flash_write and ... which guarantee that addr is 4-byte aligned
 * and len is a multiple of 4.
 */
void jshFlashWrite(
    void *buf,     //!< Buffer to write from
    uint32_t addr, //!< Flash address to write into
    uint32_t len   //!< Length of data to write
  ) {
#if ESP_IDF_VERSION_MAJOR>=5
  esp_flash_write(NULL, addr, buf, len);
#else
  spi_flash_write(addr, buf, len);
#endif
}


/**
 * Return start address and size of the flash page the given address resides in.
 * Returns false if no page.
 */
bool jshFlashGetPage(
    uint32_t addr,       //!<
    uint32_t *startAddr, //!<
    uint32_t *pageSize   //!<
  ) {
  if (addr >= FLASH_MAX) return false;
  *startAddr = addr & ~(FLASH_PAGE-1);
  *pageSize = FLASH_PAGE;
  return true;
}

void addFlashArea(JsVar *jsFreeFlash, uint32_t addr, uint32_t length) {
  JsVar *jsArea = jsvNewObject();
  if (!jsArea) return;
  jsvObjectSetChildAndUnLock(jsArea, "addr", jsvNewFromInteger((JsVarInt)addr));
  jsvObjectSetChildAndUnLock(jsArea, "length", jsvNewFromInteger((JsVarInt)length));
  jsvArrayPushAndUnLock(jsFreeFlash, jsArea);
}

JsVar *jshFlashGetFree() {
  JsVar *jsFreeFlash = jsvNewEmptyArray();
  if (!jsFreeFlash) return 0;
  // Space reserved here in the parition table -  using sub type 0x40
  // This should be read from the partition table
  addFlashArea(jsFreeFlash, 0xE000, 0x2000);
  addFlashArea(jsFreeFlash, 0x310000, 0x10000);
  addFlashArea(jsFreeFlash, 0x360000, 0xA0000);
  return jsFreeFlash;
}


/**
 * Erase the flash page containing the address.
 */
void jshFlashErasePage(
    uint32_t addr //!<
  ) {
#if ESP_IDF_VERSION_MAJOR>=5
  esp_flash_erase_region(NULL, addr >> FLASH_PAGE_SHIFT, FLASH_PAGE);
#else
  spi_flash_erase_sector(addr >> FLASH_PAGE_SHIFT);
#endif
}

size_t jshFlashGetMemMapAddress(size_t ptr) {
   // if ptr is high already, assume we know what we're accessing
  if (ptr > 0x10000000) return ptr;
  // romdata_jscode is memory mapped address from the js_code partition in rom - targets/esp32/main.c
  extern char* romdata_jscode;
  if (romdata_jscode==0) {
    jsError("Couldn't find js_code partition - update with partition_espruino.bin\n");
    return 0;
  }
  // Flash memory access is offset to 0, so remove starting location as already accounted for
  return (size_t)&romdata_jscode[ptr - FLASH_SAVED_CODE_START ];
}

unsigned int jshSetSystemClock(JsVar *options) {
  UNUSED(options);
  jsError(">> jshSetSystemClock Not implemented");
  return 0;
}

/**
 * Convert an Espruino pin id to a native ESP32 pin id.
 */
gpio_num_t pinToESP32Pin(Pin pin) {
  if ( pin < 40 )
    return pin + GPIO_NUM_0;
  jsError( "pinToESP32Pin: Unknown pin: %d", pin);
  return -1;
}

/// Perform a proper hard-reboot of the device
void jshReboot() {
  esp_restart(); // Call the ESP-IDF to restart the ESP32.
}

/* Adds the estimated power usage of the microcontroller in uA to the 'devices' object. The CPU should be called 'CPU' */
void jsvGetProcessorPowerUsage(JsVar *devices) {
  jsvObjectSetChildAndUnLock(devices, "CPU", jsvNewFromInteger(20000));
  // standard power usage of ESP32S3 without Wifi
}

#endif // original