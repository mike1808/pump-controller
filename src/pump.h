#pragma once
#include <tinyfsm.hpp>

struct ProfileConfig {
  int yeetTime;
  int waitTime;
  int rampupTime;
  int holdTime;
  int declineTime;

  int yeetPower;
  int holdPower;
};

struct Tick : tinyfsm::Event {
  long now;
};
struct SenseOn : tinyfsm::Event {};
struct SenseOff : tinyfsm::Event {};
struct KnobPress : tinyfsm::Event {};
struct KnobChange : tinyfsm::Event {
  int position;
  int max;
};

class Pump : public tinyfsm::Fsm<Pump> {
  friend class Fsm;

public:
  void react(tinyfsm::Event const &){};

  virtual void react(SenseOn const &);
  void react(SenseOff const &);
  virtual void react(KnobPress const &);
  virtual void react(KnobChange const &);

  virtual void react(Tick const &);

  virtual void entry(void){};
  virtual void exit(void){};

protected:
  static int changePower(int);

  inline static int dutyCycle;
  inline static int maxDutyCycle;

public:
  static void setMaxDutyCycle(int duty) { maxDutyCycle = duty; };
  static int getDutyCycle() { return dutyCycle; };
};

class Idle : public Pump {
  void entry() override;
};

class Profiling : public Pump {
  void entry() override;
  void exit() override;

  void react(KnobChange const &) override;
  void react(SenseOn const &) override;
  void react(Tick const &) override;

protected:
  unsigned long start;
  bool on;
};

class Manual : public Pump {
  void react(KnobChange const &) override;
  void react(KnobPress const &) override;
};
