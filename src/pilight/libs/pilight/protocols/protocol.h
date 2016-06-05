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

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

//from ../core/options.h
#define options_add(options, id, name, argtype, conftype, vartype, def, mask)

#include "../core/json.h"

// from ../config/hardware.h
typedef enum {
  HWINTERNAL = -1,
  NONE = 0,
  RF433,
  RF868,
  RFIR,
  ZWAVE,
  SENSOR,
  HWRELAY,
  API
} hwtype_t;

typedef enum {
  FIRMWARE = -2,
  PROCESS = -1,
  RAW = 0,
  SWITCH,
  DIMMER,
  WEATHER,
  RELAY,
  SCREEN,
  CONTACT,
  PENDINGSW,
  DATETIME,
  XBMC,
  LIRC,
  WEBCAM,
  MOTION,
  DUSK,
  PING,
  LABEL,
  ALARM
} devtype_t;

typedef struct protocol_t {
  char *id;
  int rawlen;
  int minrawlen;
  int maxrawlen;
  int mingaplen;
  int maxgaplen;
  short txrpt;
  short rxrpt;
  short multipleId;
  short config;
  short masterOnly;
  struct options_t *options;
  struct JsonNode *message;

  int repeats;
  unsigned long first;
  unsigned long second;

  int *raw;

  hwtype_t hwtype;
  devtype_t devtype;
  struct protocol_devices_t *devices;
  struct protocol_threads_t *threads;

  union {
    void (*parseCode)(void);
    void (*parseCommand)(struct JsonNode *code);
  };
  int (*validate)(void);
  int (*createCode)(JsonNode *code);
  int (*checkValues)(JsonNode *code);
  struct threadqueue_t *(*initDev)(JsonNode *device);
  void (*printHelp)(void);
  void (*gc)(void);
  void (*threadGC)(void);

  /* ESPiLight special, used to compare repeated messages*/
  char *old_content;
} protocol_t;

typedef struct protocols_t {
  struct protocol_t *listener;
  char *name;
  struct protocols_t *next;
} protocols_;

extern struct protocols_t *protocols;

void protocol_init(void);
void protocol_set_id(protocol_t *proto, char *id);
void protocol_register(protocol_t **proto);
#define protocol_device_add(proto, id, desc)

#endif
