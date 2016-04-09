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
 * Contains ESP8266 board specific functions.
 * ----------------------------------------------------------------------------
 */
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>
#include <gpio.h>
#include <mem.h>
#include <espmissingincludes.h>

#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include "jsutils.h"

/**
 * Convert an ESP8266 error code to a string.
 * Given an ESP8266 network error code, return a string representation
 * of the meaning of that code.
 * \return A string representation of an error code.
 */
 
FLASH_STR(__ESPCONN_ABRT, "ESPCONN_ABRT");
FLASH_STR(__ESPCONN_ARG, "ESPCONN_ARG");
FLASH_STR(__ESPCONN_CLSD, "ESPCONN_CLSD");
FLASH_STR(__ESPCONN_CONN, "ESPCONN_CONN");
FLASH_STR(__ESPCONN_HANDSHAKE, "ESPCONN_HANDSHAKE");
FLASH_STR(__ESPCONN_INPROGRESS, "ESPCONN_INPROGRESS");
FLASH_STR(__ESPCONN_ISCONN, "ESPCONN_ISCONN");
FLASH_STR(__ESPCONN_MEM, "ESPCONN_MEM");
FLASH_STR(__ESPCONN_RST, "ESPCONN_RST");
FLASH_STR(__ESPCONN_RTE, "ESPCONN_RTE");
FLASH_STR(__ESPCONN_TIMEOUT, "ESPCONN_TIMEOUT");
FLASH_STR(__unknown_error, "Unknown error");

const char *esp8266_errorToString(
    sint8 err //!< The error code to be transformed to a string.
  ) {
  switch(err) {
  case ESPCONN_MEM:
    return __ESPCONN_MEM;
  case ESPCONN_TIMEOUT:
    return __ESPCONN_TIMEOUT;
  case ESPCONN_RTE:
    return __ESPCONN_RTE;
  case ESPCONN_INPROGRESS:
    return __ESPCONN_INPROGRESS;
  case ESPCONN_ABRT:
    return __ESPCONN_ABRT;
  case ESPCONN_RST:
    return __ESPCONN_RST;
  case ESPCONN_CLSD:
    return __ESPCONN_CLSD;
  case ESPCONN_CONN:
    return __ESPCONN_CONN;
  case ESPCONN_ARG:
    return __ESPCONN_ARG;
  case ESPCONN_ISCONN:
    return __ESPCONN_ISCONN;
  case ESPCONN_HANDSHAKE:
    return __ESPCONN_HANDSHAKE;
  default:
    return __unknown_error;
  }
}


/**
 * Write a buffer of data to the console.
 * The buffer is pointed to by the buffer
 * parameter and will be written for the length parameter.  This is useful because
 * unlike a string, the data does not have to be NULL terminated.
 */
void esp8266_board_writeString(
    uint8 *buffer, //!< The start of the buffer to write.
    size_t length  //!< The length of the buffer to write.
  ) {
  assert(length==0 || buffer != NULL);

  size_t i;
  for (i=0; i<length; i++) {
    os_printf("%c", buffer[i]);
  }
}

/**
 * A debug exposed function that is global.
 * This function is exclusively for debugging.  It allows us to insert a quick
 * debug log statement in code that will log to the ESP8266 console.  It is VITAL
 * that if a call to this is inserted into non ESP8266 code that it be removed
 * before checking in to Github.  A call to this function must NEVER be
 * exposed to non-ESP8266 code and builds.
 */
void esp8266_log(char *message) {
  os_printf("%s", message);
}
