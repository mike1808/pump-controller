#include <Arduino.h>
#include <Bounce2.h>
#include <TinyPICO.h>

#include "knob.h"
#include "pump.h"
#include "sense.h"

// #define ENABLE_SERIAL

#define MOTOR_PWM_PIN (33)
#define MOTOR_DIR_PIN (32)
#define MOTOR_PWM_FREQUENCY 10000
#define MOTOR_PWM_BITS 10
#define MOTOR_PWM_CHANNEL 0

#define PIN_IN1 (25)
#define PIN_IN2 (26)
#define PIN_BUTTON (27)
#define SENSE_PIN (4)

void debugStates();

Sense sense(1, SENSE_PIN);
TinyPICO tp = TinyPICO();

int motorMaxDutyCycle = (1 << MOTOR_PWM_BITS) - 1;
int maxTicks = 120;

Knob *knob = nullptr;
IRAM_ATTR void checkPosition() {
  knob->tick(); // just call tick() to check the state.
}

void setup() {
#ifdef ENABLE_SERIAL
  Serial.begin(115200);
#endif // ENNABLE_SERIAL

  knob = new Knob(PIN_IN1, PIN_IN2, PIN_BUTTON, 0, maxTicks);
  attachInterrupt(digitalPinToInterrupt(PIN_IN1), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_IN2), checkPosition, CHANGE);

  pinMode(MOTOR_DIR_PIN, OUTPUT);
  pinMode(MOTOR_PWM_PIN, OUTPUT);
  ledcSetup(MOTOR_PWM_CHANNEL, MOTOR_PWM_FREQUENCY, MOTOR_PWM_BITS);
  ledcAttachPin(MOTOR_PWM_PIN, MOTOR_PWM_CHANNEL);
  ledcWrite(MOTOR_PWM_CHANNEL, 0);

  pinMode(SENSE_PIN, INPUT);

  tp.DotStar_SetPower(true);
  tp.DotStar_SetBrightness(126);

  // set the dirction
  digitalWrite(MOTOR_DIR_PIN, LOW);

  Pump::start();
  Pump::setMaxDutyCycle(motorMaxDutyCycle);
}

void loop() {
  Tick tick;
  tick.now = millis();

  static int currentKnobPosition = 0;

  knob->tick();
  sense.update();

  if (knob->getPosition() != currentKnobPosition) {
    currentKnobPosition = knob->getPosition();
    KnobChange knobChange;
    knobChange.max = maxTicks;
    knobChange.position = currentKnobPosition;
    Pump::dispatch(knobChange);
  }

  if (sense.changed) {
    if (sense.isOn()) {
      Pump::dispatch(SenseOn());
    } else {
      currentKnobPosition = 0;
      knob->setPosition(0);
      Pump::dispatch(SenseOff());
    }
  }

  if (knob->button->pressed()) {
    Pump::dispatch(KnobPress());
  }

  Pump::dispatch(tick);

  debugStates();

  ledcWrite(MOTOR_PWM_CHANNEL, Pump::getDutyCycle());
}

void debugStates() {
  if (Pump::is_in_state<Idle>()) {
    tp.DotStar_SetPixelColor(255, 255, 255);
  } else if (Pump::is_in_state<Profiling>()) {
    tp.DotStar_SetPixelColor(255, 0, 0);
  } else if (Pump::is_in_state<Manual>()) {
    tp.DotStar_SetPixelColor(0, 255, 0);
  }
}