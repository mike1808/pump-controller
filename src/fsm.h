#pragma once

#include "pump.h"
#include <tinyfsm.hpp>

template <typename E> void send_event(E const &event) {
  Pump::template dispatch<E>(event);
}