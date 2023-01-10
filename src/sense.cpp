#include "sense.h"
#include <Arduino.h>

Sense::Sense(int interval, int pin) : _interval(interval), _pin(pin) {}

bool Sense::update() {
  unsigned long now = millis();
  if (now - _lastCheck >= _interval) {
    bool _state = _readSample();
    changed = _lastState != _state;
    _lastState = _state;
    _lastCheck = now;
  }

  return isOn();
}

bool Sense::isOn() { return _samples; }

bool Sense::isOff() { return isOn(); }

bool Sense::_readSample() {
  _samples |= 1 - digitalRead(_pin);
  _samples <<= 1;
  return _samples;
}