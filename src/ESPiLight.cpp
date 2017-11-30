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

#include <ESPiLight.h>

extern "C" {
#include "pilight/libs/pilight/protocols/protocol.h"
}
struct protocols_t *protocols = NULL;

volatile PulseTrain_t ESPiLight::_pulseTrains[RECEIVER_BUFFER_SIZE];
boolean ESPiLight::_enabledReceiver;
volatile int ESPiLight::_actualPulseTrain = 0;
int ESPiLight::_avaiablePulseTrain = 0;
volatile unsigned long ESPiLight::_lastChange =
    0;  // Timestamp of previous edge
volatile uint8_t ESPiLight::_nrpulses = 0;

unsigned int ESPiLight::minrawlen = 5;
unsigned int ESPiLight::maxrawlen = MAXPULSESTREAMLENGTH;
unsigned int ESPiLight::mingaplen = 5100;
unsigned int ESPiLight::maxgaplen = 10000;

static void fire_callback(protocol_t *protocol, ESPiLightCallBack callback);

void ESPiLight::initReceiver(byte inputPin) {
  int interrupt = digitalPinToInterrupt(inputPin);

  resetReceiver();
  enableReceiver();

  if (interrupt >= 0) {
    attachInterrupt(interrupt, interruptHandler, CHANGE);
  }
}

int ESPiLight::receivePulseTrain(uint16_t *pulses) {
  int i = 0;
  int length = nextPulseTrainLength();

  if (length > 0) {
    volatile PulseTrain_t &pulseTrain = _pulseTrains[_avaiablePulseTrain];
    _avaiablePulseTrain = (_avaiablePulseTrain + 1) % RECEIVER_BUFFER_SIZE;
    for (i = 0; i < length; i++) {
      pulses[i] = pulseTrain.pulses[i];
    }
    pulseTrain.length = 0;
  }
  return length;
}

int ESPiLight::nextPulseTrainLength() {
  return _pulseTrains[_avaiablePulseTrain].length;
}

void ICACHE_RAM_ATTR ESPiLight::interruptHandler() {
  if (!_enabledReceiver) {
    return;
  }

  unsigned long now = micros();
  unsigned int duration = 0;

  volatile PulseTrain_t &pulseTrain = _pulseTrains[_actualPulseTrain];
  volatile uint16_t *codes = pulseTrain.pulses;

  if (pulseTrain.length == 0) {
    duration = now - _lastChange;
    /* We first do some filtering (same as pilight BPF) */
    if (duration > MIN_PULSELENGTH) {
      if (duration < MAX_PULSELENGTH) {
        /* All codes are buffered */
        codes[_nrpulses] = duration;
        _nrpulses = (_nrpulses + 1) % MAXPULSESTREAMLENGTH;
        /* Let's match footers */
        if (duration > mingaplen) {
          // Serial.print('g');
          /* Only match minimal length pulse streams */
          if (_nrpulses >= minrawlen && _nrpulses <= maxrawlen) {
            // Serial.print(_nrpulses);
            // Serial.print('l');
            pulseTrain.length = _nrpulses;
            _actualPulseTrain = (_actualPulseTrain + 1) % RECEIVER_BUFFER_SIZE;
          }
          _nrpulses = 0;
        }
      }
      _lastChange = now;
    }
  } else {
    Serial.print("_!_");
  }
  return;
}

void ESPiLight::resetReceiver() {
  int i = 0;
  for (i = 0; i < RECEIVER_BUFFER_SIZE; i++) {
    _pulseTrains[i].length = 0;
  }
  _actualPulseTrain = 0;
  _nrpulses = 0;
}

void ESPiLight::enableReceiver() { _enabledReceiver = true; }

void ESPiLight::disableReceiver() { _enabledReceiver = false; }

void ESPiLight::loop() {
  int length = 0;
  uint16_t pulses[MAXPULSESTREAMLENGTH];

  length = receivePulseTrain(pulses);

  if (length > 0) {
    /*
    Serial.print("RAW (");
    Serial.print(length);
    Serial.print("): ");
    for(int i=0;i<length;i++) {
      Serial.print(pulses[i]);
      Serial.print(' ');
    }
    Serial.println();
    */
    parsePulseTrain(pulses, length);
  }
}

ESPiLight::ESPiLight(int8_t outputPin) {
  _outputPin = outputPin;
  _callback = NULL;
  _rawCallback = NULL;

  if (_outputPin >= 0) {
    pinMode(_outputPin, OUTPUT);
    digitalWrite(_outputPin, LOW);
  }

  if (protocols == NULL) protocol_init();
}

void ESPiLight::setCallback(ESPiLightCallBack callback) {
  _callback = callback;
}

void ESPiLight::setPulseTrainCallBack(PulseTrainCallBack rawCallback) {
  _rawCallback = rawCallback;
}

void ESPiLight::sendPulseTrain(const uint16_t *pulses, int length,
                               int repeats) {
  // boolean receiverState = _enabledReceiver;
  int r = 0, x = 0;
  if (_outputPin >= 0) {
    // disableReceiver()
    for (r = 0; r < repeats; r++) {
      for (x = 0; x < length; x += 2) {
        digitalWrite(_outputPin, HIGH);
        delayMicroseconds(pulses[x]);
        digitalWrite(_outputPin, LOW);
        if (x + 1 < length) {
          delayMicroseconds(pulses[x + 1]);
        }
      }
    }
    digitalWrite(_outputPin, LOW);
    // if (receiverState) enableReceiver();
  }
  return;
}

int ESPiLight::send(const String &protocol, const String &json, int repeats) {
  int length = 0;
  uint16_t pulses[MAXPULSESTREAMLENGTH];

  length = createPulseTrain(pulses, protocol, json);
  if (length > 0) {
    /*
    Serial.println();
    Serial.print("send: ");
    Serial.print(length);
    Serial.print(" pulses (");
    Serial.print(protocol);
    Serial.print(", ");
    Serial.print(content);
    Serial.println(")");
    */
    sendPulseTrain(pulses, length, repeats);
  }
  return length;
}

int ESPiLight::createPulseTrain(uint16_t *pulses, const String &protocol_id,
                                const String &content) {
  struct protocol_t *protocol = NULL;
  struct protocols_t *pnode = protocols;
  int return_value = EXIT_FAILURE;
  JsonNode *message;

  Serial.print("piLightCreatePulseTrain: ");

  if (json_validate(content.c_str()) != true) {
    Serial.print("invalid json: ");
    Serial.println(content);
    return -2;
  }

  while (pnode != NULL) {
    protocol = pnode->listener;

    if ((protocol->createCode != NULL) && (protocol_id == protocol->id) &&
        (protocol->maxrawlen <= MAXPULSESTREAMLENGTH)) {
      Serial.print("protocol: ");
      Serial.print(protocol->id);

      protocol->rawlen = 0;
      protocol->raw = pulses;
      message = json_decode(content.c_str());
      return_value = protocol->createCode(message);
      json_delete(message);
      // delete message created by createCode()
      json_delete(protocol->message);
      protocol->message = NULL;

      if (return_value == EXIT_SUCCESS) {
        Serial.println(" create Code succeded.");
        return protocol->rawlen;
      } else {
        Serial.println(" create Code failed.");
        return -1;
      }
    }
    pnode = pnode->next;
  }
  return 0;
}

int ESPiLight::parsePulseTrain(uint16_t *pulses, int length) {
  int matches = 0;
  struct protocol_t *protocol = NULL;
  struct protocols_t *pnode = protocols;

  // Serial.println("piLightParsePulseTrain start");
  while ((pnode != NULL) && (_callback != NULL)) {
    protocol = pnode->listener;

    if (protocol->parseCode != NULL && protocol->validate != NULL) {
      protocol->raw = pulses;
      protocol->rawlen = length;

      if (protocol->validate() == 0) {
        Serial.print("pulses: ");
        Serial.print(length);
        Serial.print(" possible protocol: ");
        Serial.println(protocol->id);

        if (protocol->first > 0) {
          protocol->first = protocol->second;
        }
        protocol->second = micros();
        if (protocol->first == 0) {
          protocol->first = protocol->second;
        }

        /* Reset # of repeats after a certain delay */
        if (((int)protocol->second - (int)protocol->first) > 500000) {
          protocol->repeats = 0;
        }

        protocol->message = NULL;
        protocol->parseCode();
        if (protocol->message != NULL) {
          matches++;
          protocol->repeats++;

          fire_callback(protocol, _callback);

          json_delete(protocol->message);
          protocol->message = NULL;
        }
      }
    }
    pnode = pnode->next;
  }
  if (_rawCallback != NULL) {
    (_rawCallback)(pulses, length);
  }

  // Serial.print("piLightParsePulseTrain end. matches: ");
  // Serial.println(matches);
  return matches;
}

static void fire_callback(protocol_t *protocol, ESPiLightCallBack callback) {
  PilightRepeatStatus_t status = FIRST;
  char *content = json_encode(protocol->message);
  String deviceId = "";
  double itmp;
  char *stmp;

  if ((protocol->repeats <= 1) || (protocol->old_content == NULL)) {
    status = FIRST;
    json_free(protocol->old_content);
    protocol->old_content = content;
  } else if (protocol->repeats < 100) {
    if (strcmp(content, protocol->old_content) == 0) {
      protocol->repeats += 100;
      status = VALID;
    } else {
      status = INVALID;
    }
    json_free(protocol->old_content);
    protocol->old_content = content;
  } else {
    status = KNOWN;
    json_free(content);
  }
  if (json_find_number(protocol->message, "id", &itmp) == 0) {
    deviceId = String((int)round(itmp));
  } else if (json_find_string(protocol->message, "id", &stmp) == 0) {
    deviceId = String(stmp);
  };
  (callback)(String(protocol->id), String(protocol->old_content), status,
             protocol->repeats, deviceId);
}

String ESPiLight::pulseTrainToString(const uint16_t *codes, int length) {
  int i = 0, x = 0, match = 0;
  int diff = 0;
  int plstypes[MAX_PULSE_TYPES];
  String data("");

  if (length <= 0) {
    return String("");
  }

  for (x = 0; x < MAX_PULSE_TYPES; x++) {
    plstypes[x] = 0;
  }

  data.reserve(6 + length);
  // Serial.print("pulseTrainToString: ");
  int p = 0;
  data += "c:";
  for (i = 0; i < length; i++) {
    match = 0;
    for (x = 0; x < MAX_PULSE_TYPES; x++) {
      /* We device these numbers by 10 to normalize them a bit */
      diff = (plstypes[x] / 50) - (codes[i] / 50);
      if ((diff >= -2) && (diff <= 2)) {
        /* Write numbers */
        data += (char)('0' + ((char)x));
        match = 1;
        break;
      }
    }
    if (match == 0) {
      plstypes[p++] = codes[i];
      data += (char)('0' + ((char)(p - 1)));
      if (p >= MAX_PULSE_TYPES) {
        Serial.println("too many pulse types");
        return String("");
      }
    }
  }
  data += ";p:";
  for (i = 0; i < p; i++) {
    data += plstypes[i];
    if (i + 1 < p) {
      data += ',';
    }
  }
  data += '@';
  return data;
}

int ESPiLight::stringToPulseTrain(const String &data, uint16_t *codes,
                                  int maxlength) {
  int start = 0, end = 0, pulse_index;
  unsigned int i = 0;
  int plstypes[MAX_PULSE_TYPES];

  for (i = 0; i < MAX_PULSE_TYPES; i++) {
    plstypes[i] = 0;
  }

  int scode = data.indexOf('c') + 2;
  int spulse = data.indexOf('p') + 2;
  if (scode > 0 && (unsigned)scode < data.length() && spulse > 0 &&
      (unsigned)spulse < data.length()) {
    int nrpulses = 0;
    start = spulse;
    end = data.indexOf(',', start);
    while (end > 0) {
      plstypes[nrpulses++] = data.substring(start, end).toInt();
      start = end + 1;
      end = data.indexOf(',', start);
    }
    end = data.indexOf(';', start);
    if (end < 0) {
      end = data.indexOf('@', start);
    }
    if (end < 0) {
      return -2;
    }
    plstypes[nrpulses++] = data.substring(start, end).toInt();

    int codelen = 0;
    for (i = scode; i < data.length(); i++) {
      if ((data[i] == ';') || (data[i] == '@')) break;
      if (i >= (unsigned)maxlength) break;
      pulse_index = data[i] - '0';
      if ((pulse_index < 0) || (pulse_index >= nrpulses)) return -3;
      codes[codelen++] = plstypes[pulse_index];
    }
    return codelen;
  }
  return -1;
}
