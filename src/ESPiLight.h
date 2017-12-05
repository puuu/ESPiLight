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

#ifndef ESPILIGHT_H
#define ESPILIGHT_H

#include <Arduino.h>

#define RECEIVER_BUFFER_SIZE 10

#define MIN_PULSELENGTH 80
#define MAX_PULSELENGTH 16000
#define MAXPULSESTREAMLENGTH 255

#define MAX_PULSE_TYPES 16

enum PilightRepeatStatus_t { FIRST, INVALID, VALID, KNOWN };

typedef struct PulseTrain_t {
  uint16_t pulses[MAXPULSESTREAMLENGTH];
  uint8_t length;
} PulseTrain_t;

typedef void (*ESPiLightCallBack)(const String &protocol, const String &message,
                                  int status, int repeats,
                                  const String &deviceID);
typedef void (*PulseTrainCallBack)(const uint16_t *pulses, int length);

class ESPiLight {
 public:
  /**
   * Constructor.
   */
  ESPiLight(int8_t outputPin);

  /**
   * Transmit pulse train
   */
  void sendPulseTrain(const uint16_t *pulses, int length, int repeats = 10);

  /**
   * Transmit Pilight json message
   */
  int send(const String &protocol, const String &json, int repeats = 10);

  /**
   * Parse pulse train and fire callback
   */
  int parsePulseTrain(uint16_t *pulses, int length);

  /**
   * Process receiver queue and fire callback
   */
  void loop();

  void setCallback(ESPiLightCallBack callback);
  void setPulseTrainCallBack(PulseTrainCallBack rawCallback);

  /**
   * Initialise receiver
   */
  static void initReceiver(byte inputPin);

  /**
   * Get last received PulseTrain.
   * Returns: length of PulseTrain or 0 if not avaiable
   */
  static int receivePulseTrain(uint16_t *pulses);

  /**
   * Check if new PulseTrain avaiable.
   * Returns: 0 if no new PulseTrain avaiable
   */
  static int nextPulseTrainLength();

  /**
   * Enable Receiver. No need to call enableReceiver() after initReceiver().
   */
  static void enableReceiver();

  /**
   * Disable decoding. You can re-enable decoding by calling enableReceiver();
   */
  static void disableReceiver();

  /**
   * interruptHandler is called on every change in the input
   * signal. If RcPilight::initReceiver is called with interrupt <0,
   * you have to call interruptHandler() yourself. (Or use
   * InterruptChain)
   */
  static void interruptHandler();

  /**
   * Limit the available protocols.
   *
   * This gets a json array of the protocol names that should be activated.
   * If the array is empty, the filter gets reset.
   */
  static void limitProtocols(const String &protos);

  /**
   * Return a json array containing all the available protocols.
   */
  static String availableProtocols();

  /**
   * Return an json array containing all the currently enabled protocols
   */
  static String enabledProtocols();

  static unsigned int minrawlen;
  static unsigned int maxrawlen;
  static unsigned int mingaplen;
  static unsigned int maxgaplen;

  static String pulseTrainToString(const uint16_t *pulses, int length);
  static int stringToPulseTrain(const String &data, uint16_t *pulses,
                                int maxlength);

  static int createPulseTrain(uint16_t *pulses, const String &protocol_id,
                              const String &json);

 private:
  ESPiLightCallBack _callback;
  PulseTrainCallBack _rawCallback;
  int8_t _outputPin;

  /**
   * Quasi-reset. Called when the current edge is too long or short.
   * reset "promotes" the current edge as being the first edge of a new
   * sequence.
   */
  static void resetReceiver();

  /**
   * Internal functions
   */
  static bool _enabledReceiver;  // If true, monitoring and decoding is
                                 // enabled. If false, interruptHandler will
                                 // return immediately.
  static volatile PulseTrain_t _pulseTrains[];
  static volatile int _actualPulseTrain;
  static int _avaiablePulseTrain;
  static volatile unsigned long _lastChange;  // Timestamp of previous edge
  static volatile uint8_t _nrpulses;
};

#endif
