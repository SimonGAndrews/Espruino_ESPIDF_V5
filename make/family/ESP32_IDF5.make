#
# Definitions for the build of the ESP32 in IDF V5
#

ESP32_IDF5=1  # SGA todo may not be needed
#needed for using ifdef in wrapper JSON
DEFINES += -DESP32
DEFINES += -DESPR_DEFINES_ON_COMMANDLINE

SOURCES += targets/esp32/IDF5/main.c \
targets/esp32/IDF5/jshardware.c \
targets/esp32/IDF5/jshardwareESP32.c 

#SOURCES +=  libs/network/esp32/network_esp32.c \

SOURCES +=  targets/esp32/IDF5/jshardwareI2c.c \
 targets/esp32/IDF5/jshardwareSpi.c \
 targets/esp32/IDF5/jshardwareSpi.c \
 targets/esp32/IDF5/jshardwareUart.c \
 targets/esp32/IDF5/jshardwareAnalog.c \
 targets/esp32/IDF5/jshardwarePWM.c \
 targets/esp32/IDF5/rtosutil.c \
 targets/esp32/IDF5/jshardwarePulse.c

#WRAPPERSOURCES += \
 # libs/network/jswrap_wifi.c \
 # libs/network/esp32/jswrap_esp32_network.c \
 # targets/esp32/jswrap_esp32.c
INCLUDE += -I$(ROOT)/libs/network/esp32
 
ifdef RTOS
 # DEFINES += -DRTOS
 # WRAPPERSOURCES += targets/esp32/jswrap_rtos.c
endif # RTOS

ifeq ($(USE_NEOPIXEL),1)
SOURCES +=  targets/esp32/IDF5/esp32_neopixel.c
endif

INCLUDE += -I$(ROOT)/targets/esp32/IDF5

ifdef USE_BLUETOOTH
 SOURCES+= targets/esp32/bluetooth.c \
 targets/esp32/BLE/esp32_bluetooth_utils.c \
 targets/esp32/BLE/esp32_gap_func.c \
 targets/esp32/BLE/esp32_gatts_func.c \
 targets/esp32/BLE/esp32_gattc_func.c
endif 

