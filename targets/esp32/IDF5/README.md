# ESP32 IDF5 builds (Experimental)

The folders here contain an experimental set of files to attempt to build Espruino
for the esp32 family using the Espressif IoT Development Framework (IDF) version 5
currently at version 5.2.2.   They build on the previous work to build on IDF v 4. [^1]

Changes are documented in /docs  
Tests during development are presented in /tests

## background / history

Up until may 2024 the ESP32 build of Espruino was implemented using scripts in the [EspruinoBuildTools](https://github.com/espruino/EspruinoBuildTools/tree/master/esp32) repository.  These tools used used ESP-IDF v3.3 and made use of the Espruino makefile to build Espruino.  In ESP-IDF v4, Espressif removed the option to build ESP32 applications using make and moved solely to using cmake.  In addition espressif provided a build tool "idf.py" as a wrapper to the cmake ESP32 application builds.  
So when , in May 24 , Espruino was enhanced with a build for the ESP32-C3 it was agreed to migrate the ESP32 build method to cmake , in ESP-IDF V4 and @Gorden implemented the [change](https://github.com/espruino/Espruino/commit/57222f0915c8f4fa221afb6387b3df2d08fa3a63) for the ESP32-c3 build , but leaving the ESP32 build utilising ESP-IDF 3.3 .  See forum discussions [ESP32-C3 Mini](https://forum.espruino.com/conversations/395499/) and [and specifically here](https://forum.espruino.com/comments/17366228/) which describes the thinking.

## This Experimental approach

The changes noted here are experimental to move the build for the ESP32 directly to IDF version 5.  In doing so , we will attempt to

- enable a set of the current espressif target chips (ESP32, ESP32-c3 and ES32-s3) relying on the IDF [Project Configuration approach](https://docs.espressif.com/projects/esp-idf/en/release-v5.2/esp32/api-reference/kconfig.html) and the Espruino "Board.py" configs to minimise the use of #ifdefs in the build files. 
- create a new ESP32-V5 target Library as a clean slate without providing #ifdefs for previous IDF versions.
- build upon the cmake approach.
- provide a learning experience with reasonable notes [targets/esp32/IDF5/docs] documenting the journey.

## STATUS

### currently working on
  
- basic setup of esp idf v5 using source/provision/py script

#### Current targets under development

- [x] ESP32
- [ ] ESP32-S2
- [ ] ESP32-C3
- [ ] ESP32-S3
- [ ] ESP32-C6
- [ ] ESP32-H2

List (Draft) of toDos

- ADC
- Bluetooth
- BLE
- DAC
- Ethernet
- GPIO
- Hall Sensor
- I2C
- I2S
- LEDC
- Motor PWM
- Pulse Counter
- RMT
- SDIO
- SDMMC
- Timer
- Temp. Sensor
- Touch
- TWAI
- UART
- USB
- ESP32-C3 only CDC/JTAG
- Wi-Fi

[^1]: [Github source for Espruino for ESP32 in version 4 of IDF][def]

[def]: https://github.com/espruino/Espruino/tree/master/targets/esp32/IDF4
