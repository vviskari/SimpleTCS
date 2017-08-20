#include <pebble-events/pebble-events.h>
#include <pebble.h>

#include "battery.h"
#include "calendar.h"
#include "datetime.h"
#include "global.h"
#include "settings.h"
#include "utils.h"
#include "weather.h"

Settings settings;
static EventHandle handle;

static void save_settings() {
  persist_delete(SETTINGS_KEY);
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void conf_inbox_received_handler(DictionaryIterator *iter, void *context) {
  bool updateWeather = false;

  Tuple *conf = dict_find(iter, MESSAGE_KEY_wsd);
  if (conf) {
    char old = settings.weekStartDay;
    settings.weekStartDay = conf->value->cstring[0];
    if (old != settings.weekStartDay) {
      drawcal();
      handle_battery();
    }
  }

  conf = dict_find(iter, MESSAGE_KEY_wt);
  if (conf) {
    settings.weatherTemp = conf->value->cstring[0];
  }

  conf = dict_find(iter, MESSAGE_KEY_df);
  if (conf) {
    settings.dateFormat = conf->value->cstring[0];
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    handle_time(current_time, MINUTE_UNIT);
  }

  conf = dict_find(iter, MESSAGE_KEY_wf);
  if (conf) {
    bool old = settings.forecast;
    settings.forecast = conf->value->int8 == 1 ? true : false;
    if (old != settings.forecast) {
      updateWeather = true;
      if (!settings.forecast) {
        set_show_forecast(false);
      }
    }
  }

  conf = dict_find(iter, MESSAGE_KEY_vm);
  if (conf) {
    settings.viewMode = conf->value->cstring[0];
    if (settings.viewMode == 'c') {
      set_show_forecast(false);
    } else if (settings.viewMode == 'f') {
      set_show_forecast(true);
    }
  }

  conf = dict_find(iter, MESSAGE_KEY_wp);
  if (conf) {
    char old = settings.weatherProvider;
    settings.weatherProvider = conf->value->cstring[0];
    if (old != settings.weatherProvider) {
      updateWeather = true;
    }
    if (settings.weatherProvider != 'w') {
      settings.forecast = false;
      set_show_forecast(false);
    }
  }

  conf = dict_find(iter, MESSAGE_KEY_wak);
  if (conf) {
    char old[20];
    strncpy(old, settings.weatherApiKey, 20);
    strncpy(settings.weatherApiKey, conf->value->cstring, 20);
    if (strcmp(old, settings.weatherApiKey) != 0) {
      updateWeather = true;
    }
  }

  save_settings();
  handle_weather(updateWeather);
}

void load_settings() {
  settings.weekStartDay = 'm';
  settings.weatherTemp = 'C';
  settings.weatherProvider = 'y';
  settings.forecast = false;
  settings.viewMode = 's';    // s, t, c, f
  settings.dateFormat = '1';  // 1, 2, 3

  if (persist_exists(SETTINGS_KEY)) {
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  }
}

void settings_init() {
  // Clay
  handle = events_app_message_register_inbox_received(conf_inbox_received_handler, NULL);
}

void settings_deinit() {
  events_app_message_unsubscribe(handle);
}
