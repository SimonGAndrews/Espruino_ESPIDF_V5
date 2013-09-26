/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Misc utils and cheapskate stdlib implementation
 * ----------------------------------------------------------------------------
 */
#include "jsutils.h"
#include "jslex.h"
#include "jshardware.h"
#include "jsinteractive.h"

bool isIDString(const char *s) {
    if (!isAlpha(*s))
        return false;
    while (*s) {
        if (!(isAlpha(*s) || isNumeric(*s)))
            return false;
        s++;
    }
    return true;
}

/** escape a character - if it is required. This may return a reference to a static array, 
so you can't store the value it returns in a variable and call it again. */
const char *escapeCharacter(char ch) {
  if (ch=='\b') return "\\b";
  if (ch=='\f') return "\\f";
  if (ch=='\n') return "\\n";
  if (ch=='\a') return "\\a";
  if (ch=='\r') return "\\r";
  if (ch=='\t') return "\\t";
  if (ch=='\\') return "\\\\";
  if (ch=='"') return "\\\"";
  static char buf[5];
  if (ch<32) {  
    /** just encode as hex - it's more understandable
     * and doesn't have the issue of "\16"+"1" != "\161" */
    buf[0]='\\';
    buf[1]='x';
    int n = (ch>>4)&15;
    buf[2] = (char)((n<10)?('0'+n):('A'+n-10));
    n=ch&15;
    buf[3] = (char)((n<10)?('0'+n):('A'+n-10));
    buf[4] = 0;
    return buf;
  }
  buf[1] = 0;
  buf[0] = ch;
  return buf;
}

/* convert a number in the given radix to an int. if radix=0, autodetect */
JsVarInt stringToIntWithRadix(const char *s, JsVarInt forceRadix) {
  bool isNegated = false;
  JsVarInt v = 0;
  JsVarInt radix = 10;
  if (*s == '-') {
    isNegated = true;
    s++;
  }
  if (forceRadix == 0) {
    if (*s == '0') {
      radix = 8;
      s++;
    }
    if (*s == 'x') {
      radix = 16;
      s++;
    } else if (*s == 'b') {
      radix = 2;
      s++;
    }
  } else {
    radix = forceRadix;
  }

  while (*s) {
    if (*s >= '0' && *s <= '9')
      v = (v*radix) + (*s - '0');
    else if (*s >= 'a' && *s <= 'f')
      v = (v*radix) + (10 + *s - 'a');
    else if (*s >= 'A' && *s <= 'F')
      v = (v*radix) + (10 + *s - 'A');
    else break;
    s++;
  }

  if (isNegated) return -v;
  return v;
}

/* convert hex, binary, octal or decimal string into an int */
JsVarInt stringToInt(const char *s) {
    return stringToIntWithRadix(s,0);
}

void jsError(const char *message) {
  jsiConsoleRemoveInputLine();
  jsiConsolePrint("ERROR: ");
  jsiConsolePrint(message);
  jsiConsolePrint("\n");
}

void jsErrorAt(const char *message, struct JsLex *lex, int tokenPos) {
  jsiConsoleRemoveInputLine();
  jsiConsolePrint("ERROR: ");
  jsiConsolePrint(message);
  jsiConsolePrint(" at ");
  jsiConsolePrintPosition(lex, tokenPos);
  jsiConsolePrintTokenLineMarker(lex, tokenPos);
}

void jsWarn(const char *message) {
  jsiConsoleRemoveInputLine();
  jsiConsolePrint("WARNING: ");
  jsiConsolePrint(message);
  jsiConsolePrint("\n");
}

void jsWarnAt(const char *message, struct JsLex *lex, int tokenPos) {
  jsiConsoleRemoveInputLine();
  jsiConsolePrint("WARNING: ");
  jsiConsolePrint(message);
  jsiConsolePrint(" at ");
  jsiConsolePrintPosition(lex, tokenPos);
}

void jsAssertFail(const char *file, int line, const char *expr) {
  jsiConsoleRemoveInputLine();
  if (expr) {
    jsiConsolePrint("ASSERT(");
    jsiConsolePrint(expr);
    jsiConsolePrint(") FAILED AT ");
  } else
    jsiConsolePrint("ASSERT FAILED AT ");
  jsiConsolePrint(file);
  jsiConsolePrint(":");
  jsiConsolePrintInt(line);
  jsiConsolePrint("\n");

  jsvTrace(jsvGetRef(jsvFindOrCreateRoot()), 2);
  exit(1);
}

#ifdef SDCC
void exit(int errcode) {dst;
    jsiConsolePrint("EXIT CALLED.\n");
}
#endif

#ifdef FAKE_STDLIB
int __errno;

void exit(int errcode) {
    jsiConsolePrint("EXIT CALLED.\n");
    while (1);
}

char * strncat(char *dst, const char *src, size_t c) {
        char *dstx = dst;
        while (*(dstx++)) c--;
        while (*src && c>1) {
          *(dstx++) = *(src++);
          c--;
        }
        if (c>0) *dstx = 0;
        return dst;
}
char *strncpy(char *dst, const char *src, size_t c) {
        char *dstx = dst;
        while (*src && c>1) {
          *(dstx++) = *(src++);
          c--;
        }
        if (c>0) *dstx = 0;
        return dst;
}
size_t strlen(const char *s) {
        size_t l=0;
        while (*(s++)) l++;
        return l;
}
int strcmp(const char *a, const char *b) {
        while (*a && *b) {
                if (*a != *b)
                        return *a - *b; // correct?
                a++;b++;
        }
        return *a - *b;
}
void *memcpy(void *dst, const void *src, size_t size) {
        size_t i;
        for (i=0;i<size;i++)
                ((char*)dst)[i] = ((char*)src)[i];
        return dst;
}
unsigned int rand() { 
    static unsigned int m_w = 0xDEADBEEF;    /* must not be zero */
    static unsigned int m_z = 0xCAFEBABE;    /* must not be zero */

    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}

JsVarFloat atof(const char *s) {
  bool isNegated = false;
  bool hasDot = false;
  JsVarFloat v = 0;
  JsVarFloat mul = 0.1;
  if (*s == '-') { 
    isNegated = true;
    s++;
  }
  while (*s) {
    if (!hasDot) { 
      if (*s == '.') 
        hasDot = true;
      else if (*s >= '0' && *s <= '9')
        v = (v*10) + (*s - '0');
      else if (*s >= 'a' && *s <= 'f')
        v = (v*10) + (10 + *s - 'a');
      else if (*s >= 'A' && *s <= 'F')
        v = (v*10) + (10 + *s - 'A');
      else break;
    } else {
      if (*s >= '0' && *s <= '9')
        v += mul*(*s - '0');
      else if (*s >= 'a' && *s <= 'f')
        v += mul*(10 + *s - 'a');
      else if (*s >= 'A' && *s <= 'F')
        v += mul*(10 + *s - 'A');
      else break;
      mul = mul / 10;
    }
    s++;
  }

  if (isNegated) return -v;
  return v;
}

#endif
char itoch(int val) {
  if (val<10) return (char)('0'+val);
  return (char)('A'+val-10);
}

#ifndef HAS_STDLIB
void itoa(JsVarInt vals,char *str,unsigned int base) {
  JsVarIntUnsigned val;
  if (vals<0) {
    *(str++)='-';
    val = (JsVarIntUnsigned)(-vals);
  } else {
    val = (JsVarIntUnsigned)vals;
  }
  JsVarIntUnsigned d = 1;
  while (d*base <= val) d*=base;
  while (d > 1) {
    unsigned int v = (unsigned int)(val / d);
    val -= v*d;
    *(str++) = itoch((int)v);
    d /= base;
  }  
  *(str++)=itoch((int)val);
  *(str++)=0;
}
#endif

void ftoa(JsVarFloat val,char *str) {
  const JsVarFloat base = 10;
  if (val<0) {
    *(str++)='-';
    val = -val;
  }
  JsVarFloat d = 1;
  while (d*base <= val) d*=base;
  while (d >= 1) {
    int v = (int)(val / d);
    val -= v*d;
    *(str++)=itoch(v);
    d /= base;
  }  
#ifndef USE_NO_FLOATS
  if (val>0) {
    *(str++)='.';
    while (val>0.000001) {
      int v = (int)((val / d) + 0.0000005);
      val -= v*d;
      *(str++)=itoch(v);
      d /= base;
    }
  }
#endif

  *(str++)=0;
}


/// Wrap a value so it is always between 0 and size (eg. wrapAround(angle, 360))
JsVarFloat wrapAround(JsVarFloat val, JsVarFloat size) {
  val = val / size;
  val = val - (int)val;
  return val * size;
}
