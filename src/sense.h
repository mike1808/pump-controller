#pragma once

class Sense {
public:
  Sense(int interval, int pin);

  bool update();

  bool isOn();
  bool isOff();

  bool changed;

private:
  bool _readSample();

  int _pin;
  int _interval;

  unsigned long _lastCheck;
  bool _lastState;

  unsigned long _samples;
};