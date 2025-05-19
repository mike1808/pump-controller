#include "Arduino.h"

#include "fsm.h"
#include "pump.h"

#define YEET_TIME 1500
#define WAIT_TIME 1000
#define RAMPUP_TIME 10000
#define HOLD_TIME 5000
#define DECLINE_TIME 10000

#define YEET_POWER 60l
#define HOLD_POWER 95l

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

int Pump::changePower(int power) {
  dutyCycle = min(static_cast<int>(map(max(power, 0), 0, 100, 0, maxDutyCycle)),
                  maxDutyCycle);
  return dutyCycle;
}

void Pump::react(SenseOn const &e) {
  // ignore
}

void Pump::react(Tick const &e) {
  // ignore
}

void Pump::react(SenseOff const &e) { transit<Idle>(); }

void Pump::react(KnobChange const &e) { transit<Manual>(); }

void Pump::react(KnobPress const &e) { transit<Manual>(); }

void Idle::entry() {
  changePower(0);
  transit<Profiling>();
}

void Profiling::entry() {
  on = false;
  start = 0;
}

void Profiling::exit() {
  on = false;
  start = 0;
}

void Profiling::react(KnobChange const &e) { transit<Manual>(); }

void Profiling::react(SenseOn const &e) {
  on = true;
  start = millis();
}

void Profiling::react(Tick const &e) {
  if (!on)
    return;

  int power = getProfilePosition(e.now - start);
  changePower(power);
}

void Manual::react(KnobChange const &e) {
  int power = max(0, static_cast<int>(map(e.position, 0, e.max, 0, 100)));
  changePower(power);
}

void Manual::react(KnobPress const &e) { transit<Profiling>(); }

FSM_INITIAL_STATE(Pump, Idle);