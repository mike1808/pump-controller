#include <Arduino.h>
#include <Bounce2.h>
#include <TinyPICO.h>

#include "knob.h"

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

Bounce2::Button *sense = nullptr;
TinyPICO tp = TinyPICO();

int motorMaxDutyCycle = (1 << MOTOR_PWM_BITS) - 1;
int maxTicks = 60;
int dutyCyclePerTick =
    static_cast<int>(static_cast<float>(motorMaxDutyCycle) / maxTicks);

int senseTicks = static_cast<int>(static_cast<float>(maxTicks) * 0.6);

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

  sense = new Bounce2::Button();
  sense->attach(SENSE_PIN, INPUT_PULLUP);
  sense->interval(5);
  sense->setPressedState(LOW);

  tp.DotStar_SetPower(true);
  tp.DotStar_SetPixelColor(255, 0, 0);
  tp.DotStar_SetBrightness(126);
}

#define YEET_TIME 2000
#define WAIT_TIME 1000
#define RAMPUP_TIME 8000
#define HOLD_TIME 8000
#define DECLINE_TIME 10000

#define YEET_POWER 40l
#define HOLD_POWER 60l

int getProfilePosition(long t) {
  if (t < YEET_TIME) {
    return YEET_POWER;
  }
  t -= YEET_TIME;

  if (t < WAIT_TIME) {
    return 0;
  }
  t -= WAIT_TIME;

  if (t < RAMPUP_TIME) {
    return min(map(t, 0, RAMPUP_TIME, 0, HOLD_POWER), HOLD_POWER);
  }
  t -= RAMPUP_TIME;

  if (t < HOLD_TIME) {
    return HOLD_POWER;
  }
  t -= HOLD_TIME;

  return max(map(t, 0, DECLINE_TIME, HOLD_POWER, 0), 0l);
}

void loop() {
  static int duty = 0;
  static int pos = 0;
  static long profilingStart = 0;
  static bool profiling = false;

  digitalWrite(MOTOR_DIR_PIN, LOW);

  knob->tick();
  sense->update();

  int newPos = knob->getPosition();
  bool isSenseOn = sense->isPressed();

  if (sense->changed()) {
    if (isSenseOn) {
      if (!profiling && newPos != 0) {
        newPos = 95;
        knob->setPosition(95);
      } else {
        profilingStart = millis();
        profiling = true;
      }
    } else {
      newPos = 0;
      knob->setPosition(0);
      profiling = false;
    }
  }

  if (knob->button->pressed()) {
    if (profiling || newPos != 0) {
      newPos = 0;
      knob->setPosition(0);
      profiling = false;
    } else {
      profilingStart = millis();
      profiling = true;
    }
  }

  if (profiling) {
    newPos = getProfilePosition(millis() - profilingStart);
    knob->setPosition(newPos);
  }

  if (isSenseOn) {
    if (profiling) {
      tp.DotStar_SetPixelColor(0, 255, 255);
    } else {
      tp.DotStar_SetPixelColor(0, 0, 255);
    }
  } else if (profiling) {
    tp.DotStar_SetPixelColor(0, 255, 0);
  } else if (newPos != 0) {
    tp.DotStar_SetPixelColor(255, 255, 0);
  } else {
    tp.DotStar_SetPixelColor(255, 0, 0);
  }

  if (pos != newPos) {
#ifdef ENABLE_SERIAL
    Serial.print("pos:");
    Serial.print(newPos);
    Serial.print(" dir:");
    Serial.print((knob->getDirection()));
#endif // ENABLE_SERIAL
    pos = newPos;
    duty = min(max(pos, 0) * dutyCyclePerTick, motorMaxDutyCycle);

#ifdef ENABLE_SERIAL
    Serial.printf(" duty: %d", duty);
    Serial.println();
#endif // ENABLE_SERIAL
  }

  ledcWrite(MOTOR_PWM_CHANNEL, duty);
}
