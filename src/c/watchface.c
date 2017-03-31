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
uint32_t FORECAST_TOGGLE_KEY = 5;
uint32_t WEATHER_KEY_FORECAST = 1000;

static int secondticks = 0;
static int shakes = 0;
static bool show_seconds = false;
static bool show_forecast = false;
static int shaketicks = 0;
static int MAX_SECONDS = 30;

void set_show_forecast(bool show) {
  show_forecast = show;
  hide_forecast(!show_forecast);
  hide_calendar(show_forecast);
  hide_battery_estimate(show_forecast);
}

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

  if (tick_time->tm_sec == 0) {
    handle_minute_tick(tick_time, units_changed);
  }
  if (shaketicks % 2 == 0) {
    if (shakes == 2) {
      show_seconds = !show_seconds;
      hide_weather(show_seconds);
      hide_seconds(!show_seconds);
      secondticks = show_seconds ? 1 : MAX_SECONDS+1;
    }
    if (shakes == 3 && settings.forecast) {
      shakes = 0;
      // toggle forecast
      set_show_forecast(!show_forecast);
    }
    shaketicks = shakes = 0;
    if (!show_seconds) {
      tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
      return;
    }
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
  shaketicks = 0;
  APP_LOG(APP_LOG_LEVEL_INFO, "TAPS: %d", shakes);
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

static void load_window_state() {
  if (persist_exists(FORECAST_TOGGLE_KEY)) {
    set_show_forecast(persist_read_bool(FORECAST_TOGGLE_KEY));
  }
}

static void save_window_state() {
  persist_write_bool(FORECAST_TOGGLE_KEY, show_forecast);
}

static void main_window_load(Window *window) {
  bluetooth_load();
  datetime_load();
  calendar_load();
  battery_load();
  weather_load();
  steps_load();
  load_window_state();
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
  save_window_state();
  
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
