/*
  ESPiLight - pilight 433.92 MHz protocols library for Arduino
  Copyright (c) 2016 Puuu.  All right reserved.

  Project home: https://github.com/puuu/espilight/
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with library. If not, see <http://www.gnu.org/licenses/>
*/

#ifndef _APRINTF_H_
#define _APRINTF_H_

#include <stdio.h>

#ifndef __cplusplus
#include <pgmspace.h>
#define FSTR(s)                            \
  (__extension__({                         \
    static const char __c[] PROGMEM = (s); \
    &__c[0];                               \
  }))
#define printf(fmt, ...) printf(FSTR(fmt), ##__VA_ARGS__)
#define fprintf(stream, args...) printf(args)
#else
#define fprintf(stream, args...) printf(args)
#endif

#ifdef __cplusplus
extern "C" {
#endif
void exit(int n);
#ifdef __cplusplus
}
#endif

#endif  //_APRINTF_H_
