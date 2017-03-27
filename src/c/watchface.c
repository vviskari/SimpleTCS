//
// Simple TCS watchface for Pebble (Time series)
//
// Based on "Simple Time and Calendar" by Michael S.
// 

#include <pebble.h>
#include <pebble-generic-weather/pebble-generic-weather.h>
#include <pebble-events/pebble-events.h>

#include "global.h"
#include "utils.h"
#include "bluetooth.h"
#include "weather.h"
#include "datetime.h"
#include "steps.h"
#include "settings.h"
#include "calendar.h"
#include "battery.h"

// Globals
Window *s_main_window;
uint32_t BATT_HISTORY_KEY = 1;
uint32_t BATT_CHARGING_KEY = 2;
uint32_t WEATHER_KEY = 3;
uint32_t SETTINGS_KEY = 4;

static int secondticks = 0;
static int shakes = 0;
static bool show_seconds = false;
static bool hidden_forecast = true;
static int shaketicks = 0;
static int MAX_SECONDS = 30;

static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
  handle_time(tick_time, units_changed);
  if (tick_time->tm_sec == 0) {
    if (tick_time->tm_min == 0){ 
      // Draw calendar every hour on the hour
      drawcal();
      if (tick_time->tm_hour == 0) {
        // refresh battery stats at midnight
        handle_battery(battery_state_service_peek());
      }
    }
    // Steps every 5 mins
    if(tick_time->tm_min % 5 == 0) {
      handle_steps();
    }
    // Update weather twice an hour
    if(tick_time->tm_min % 30 == 0) {
      handle_weather(true);
    }
  }
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  secondticks++;
  shaketicks++;
  APP_LOG(APP_LOG_LEVEL_INFO,"second: shakes %d", shakes);

  if (tick_time->tm_sec == 0) {
    handle_minute_tick(tick_time, units_changed);
  }
  if (shaketicks % 2 == 0) {
    if (shakes == 2) {
      APP_LOG(APP_LOG_LEVEL_INFO,"toggle seconds");
      show_seconds = !show_seconds;
      hide_weather(show_seconds);
      hide_seconds(!show_seconds);
      secondticks = show_seconds ? 1 : MAX_SECONDS+1;
    }
    if (shakes == 3) {
      APP_LOG(APP_LOG_LEVEL_INFO,"toggle forecast");
      shakes = 0;
      hidden_forecast = !hidden_forecast;
      hide_forecast(hidden_forecast);
      hide_calendar(!hidden_forecast);
      hide_battery_estimate(!hidden_forecast);
    }
    APP_LOG(APP_LOG_LEVEL_INFO,"reset shakes");
    shaketicks = shakes = 0;
  } 

  if (show_seconds && secondticks <= MAX_SECONDS) {
    handle_seconds(tick_time);
  }

  if (secondticks > MAX_SECONDS) {
    secondticks=0;
    show_seconds = false;
    hide_seconds(true);
    hide_weather(false);
    tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  }  
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  shakes++;
  APP_LOG(APP_LOG_LEVEL_INFO,"add shakes, axis %d", axis);

  shaketicks = 0;
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

static void main_window_load(Window *window) {
  bluetooth_load();
  datetime_load();
  calendar_load();
  battery_load();
  weather_load();
  steps_load();
  shakes = 0;
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_minute_tick(current_time, MINUTE_UNIT);

  accel_tap_service_subscribe(tap_handler);
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void main_window_unload(Window *window) {

  bluetooth_unload();
  weather_unload();
  datetime_unload();
  steps_unload();
  calendar_unload();
  battery_unload();
  
  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();
}

static void init() {
  load_settings();
  weather_init();
  settings_init();

  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  weather_deinit();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
