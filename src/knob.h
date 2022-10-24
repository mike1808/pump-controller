#pragma once

#include <Bounce2.h>
#include <RotaryEncoder.h>

class Knob {
public:
  Knob(int pinA, int pinB, int pinPush);
  Knob(int pinA, int pinB, int pinPush, int min, int max);

  ~Knob();

  void useLimits(bool use);
  void setLimits(int min, int max);

  void tick();
  int getPosition();
  int getDirection();

  void setPosition(int pos);

  bool pressed();

  Bounce2::Button *button;

private:
  static void startTickTask(void *instance);
  void tickTask();

  TaskHandle_t xHandle = NULL;

  RotaryEncoder *encoder;
  bool hasLimits = false;
  int minTick;
  int maxTick;
};