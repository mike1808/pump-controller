#include "knob.h"

Knob::Knob(int pinA, int pinB, int pinPush) {
  encoder = new RotaryEncoder(pinA, pinB, RotaryEncoder::LatchMode::TWO03);

  button = new Bounce2::Button();
  button->attach(pinPush, INPUT_PULLUP);
  button->interval(5);
  button->setPressedState(LOW);

  // xTaskCreate(&Knob::startTickTask, "knobTickTask", 2048, this, 5, &xHandle);
}

Knob::Knob(int pinA, int pinB, int pinPush, int minT, int maxT)
    : Knob(pinA, pinB, pinPush) {
  hasLimits = true;
  minTick = minT;
  maxTick = maxT;
}

Knob::~Knob() {
  if (xHandle != NULL) {
    vTaskDelete(xHandle);
  }
}

void Knob::useLimits(bool use) { hasLimits = use; }

void Knob::setLimits(int minLimit, int maxLimit) {
  minTick = minLimit;
  maxLimit = maxLimit;
}

void Knob::tick() {
  encoder->tick();
  button->update();

  int pos = this->getPosition();

  // if (button->pressed() && pos > 0) {
  //   encoder->setPosition(0);
  //   return;
  // }

  if (hasLimits) {
    if (pos > maxTick) {
      encoder->setPosition(maxTick);
    } else if (pos < minTick) {
      encoder->setPosition(minTick);
    }
  }
}

void Knob::startTickTask(void *instance) { ((Knob *)instance)->tickTask(); }
void Knob::tickTask() {
  while (1) {
    this->tick();
    delay(10);
  }
}

int Knob::getPosition() { return encoder->getPosition(); }

int Knob::getDirection() { return (int)encoder->getDirection(); }

void Knob::setPosition(int pos) { return encoder->setPosition(pos); }

bool Knob::pressed() { return button->pressed(); }
