#pragma once
#include <pebble.h>

typedef struct {
  time_t timestamp;
  int charge;
  int last_estimated;
  int slope;
} BatteryHistory;

extern Window *s_main_window;
extern Layer *s_calendar_container;
extern Layer *s_forecast_container;

// Settings keys
extern uint32_t BATT_HISTORY_KEY;
extern uint32_t BATT_CHARGING_KEY;
extern uint32_t WEATHER_KEY;
extern uint32_t SETTINGS_KEY;
extern uint32_t FORECAST_TOGGLE_KEY;
extern uint32_t WEATHER_KEY_FORECAST;

#define F_HEIGHT 48
#define F_WIDTH 140
