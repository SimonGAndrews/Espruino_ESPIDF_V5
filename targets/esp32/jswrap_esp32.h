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
 * Contains ESP32 board specific function definitions.
 * ----------------------------------------------------------------------------
 */
#ifndef TARGETS_ES32_JSWRAP_ESP32_H_
#define TARGETS_ESP32_JSWRAP_ESP32_H_
#include "jsvar.h"
#include "jspin.h"

//===== ESP32 Library
JsVar *jswrap_ESP32_getState();
void   jswrap_ESP32_neopixelWrite(Pin pin, JsVar *jsArrayOfData);
void   jswrap_ESP32_setLogLevel(JsVar *jsTagToSet, JsVar *jsLogLevel);
void   jswrap_ESP32_reboot();
void   jswrap_ESP32_setAtten(Pin pin,int atten);
#endif /* TARGETS_ESP32_JSWRAP_ESP32_H_ */
