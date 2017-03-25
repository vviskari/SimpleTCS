#pragma once
#include <pebble.h>

typedef struct {
  time_t timestamp;
  int charge;
  int last_estimated;
  int slope;
} BatteryHistory;

extern Window *s_main_window;

// Settings keys
extern uint32_t BATT_HISTORY_KEY;
extern uint32_t BATT_CHARGING_KEY;
extern uint32_t WEATHER_KEY;
extern uint32_t SETTINGS_KEY;
