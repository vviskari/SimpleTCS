//
// Simple TCS watchface for Pebble (Time series)
//
// Based on "Simple Time and Calendar" by Michael S.
//

#include <pebble-events/pebble-events.h>
#include <pebble-generic-weather/pebble-generic-weather.h>
#include <pebble.h>

#include "global.h"
#include "battery.h"
#include "bluetooth.h"
#include "calendar.h"
#include "datetime.h"
#include "settings.h"
#include "steps.h"
#include "utils.h"
#include "weather.h"

// Globals
Window *s_main_window;
Layer *s_calendar_container;
Layer *s_forecast_container;

static Layer *s_animate_container;

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

void toggle_animation() {
  // animate only when weatherprovider is wunderground and viewMode is not calendar or forecast
  if (settings.weatherProvider != 'w' || settings.viewMode == 'c' || settings.viewMode == 'f') {
    return;
  }
  show_forecast = !show_forecast;

  GRect start = layer_get_frame(s_animate_container);
  GRect finish = GRect(start.origin.x, start.origin.y, start.size.w, start.size.h);

  if (show_forecast) {
    finish.origin.x = -144;
  } else {
    finish.origin.x = 0;
  }
  if (!settings.animationEnabled) {
    layer_set_frame(s_animate_container, finish);
    return;
  }
  
  PropertyAnimation *prop_anim = property_animation_create_layer_frame(s_animate_container, 
                                                                       &start, &finish);
  Animation *anim = property_animation_get_animation(prop_anim);
  const int delay_ms = 0;
  const int duration_ms = 1000;

  animation_set_curve(anim, AnimationCurveEaseOut);
  animation_set_delay(anim, delay_ms);
  animation_set_duration(anim, duration_ms);
  animation_schedule(anim);
}

void set_show_forecast(bool show) {
  show_forecast = show;
  // align animation container depending if show forecast
  GRect frame = layer_get_frame(s_animate_container);
  if (show_forecast) {
    frame.origin.x = -144;
  } else {
    frame.origin.x = 0;
  }
  layer_set_frame(s_animate_container, frame);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {

  bool from_init = false;
  if (!tick_time){
    // was called from init
    time_t now = time(NULL);
    tick_time = localtime(&now);
    from_init = true;
  }
  handle_time(tick_time, units_changed);
  if (tick_time->tm_sec == 0) {
    // animate calendar-forecast if viewMode=a
    if (settings.viewMode == 'a' && !from_init && !userIsSleeping()) {
      toggle_animation();
    }

    if (tick_time->tm_min == 0) {
      // Draw calendar every hour on the hour
      drawcal();
      if (tick_time->tm_hour == 0) {
        // refresh battery stats at midnight
        handle_battery(battery_state_service_peek());
      }
    }
    // Steps every 5 mins
    if (tick_time->tm_min % 5 == 0) {
      handle_steps();
    }
    // Update weather twice an hour
    if (tick_time->tm_min % 30 == 0) {
      handle_weather(true);
    }
  }
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  secondticks++;
  shaketicks++;

  if (tick_time->tm_sec == 0) {
    handle_minute_tick(tick_time, units_changed);
  }

  if (shaketicks % 2 == 0) {

    if (settings.viewMode != 't') {
      if (shakes == 2) {
        show_seconds = !show_seconds;
        hide_weather(show_seconds);
        hide_seconds(!show_seconds);
        secondticks = show_seconds ? 1 : MAX_SECONDS + 1;
      } else if (shakes == 3) {
        shakes = 0;
        // toggle animation
        toggle_animation();
      }

    } else if (shakes == 2) {
      // viewMode == t, shake 2 times to switch
      if (show_seconds) {
        // toggle animation if showing seconds right now
        toggle_animation();
      }
      show_seconds = !show_seconds;
      hide_weather(show_seconds);
      hide_seconds(!show_seconds);
      secondticks = show_seconds ? 1 : MAX_SECONDS + 1;
    }

    // reset shakes every 2 seconds
    shaketicks = shakes = 0;

    if (!show_seconds) {
      tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
      return;
    }    
  }

  if (show_seconds) {
    if (secondticks <= MAX_SECONDS) {
      handle_seconds(tick_time);
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "reset seconds, was %d", secondticks);
      secondticks = 0;
      show_seconds = false;
      hide_seconds(true);
      hide_weather(false);
      tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
    }
  } else {
    secondticks = 0;
  }
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  shakes++;
  shaketicks = 0;
  APP_LOG(APP_LOG_LEVEL_INFO, "TAP");
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

static void load_window_state() {
  if (persist_exists(FORECAST_TOGGLE_KEY)) {
    set_show_forecast(persist_read_bool(FORECAST_TOGGLE_KEY));
  }
}

static void save_window_state() { 
  persist_delete(FORECAST_TOGGLE_KEY);
  persist_write_bool(FORECAST_TOGGLE_KEY, show_forecast);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_main_window);
  s_animate_container = layer_create(GRect(0, 115, 288, 51));

  s_calendar_container = layer_create(GRect(0, 0, 144, 51));
  layer_add_child(s_animate_container, s_calendar_container);
  
  s_forecast_container = layer_create(GRect(144, 5, F_WIDTH, F_HEIGHT));
  layer_add_child(s_animate_container, s_forecast_container);
  
  layer_add_child(window_layer, s_animate_container);

  bluetooth_load();
  datetime_load();
  calendar_load();
  battery_load();
  weather_load();
  steps_load();
  load_window_state();
  shakes = 0;
  handle_minute_tick(NULL, MINUTE_UNIT);

  accel_tap_service_subscribe(tap_handler);
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void main_window_unload(Window *window) {
  bluetooth_unload();
  datetime_unload();

  calendar_unload();
  battery_unload();
  weather_unload();
  steps_unload();
  save_window_state();

  layer_destroy(s_calendar_container);
  layer_destroy(s_forecast_container);
  layer_destroy(s_animate_container);

  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();
}

static void init() {
  load_settings();
  weather_init();
  settings_init();
  events_app_message_open();

  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load = main_window_load, .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  weather_deinit();
  settings_deinit();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
