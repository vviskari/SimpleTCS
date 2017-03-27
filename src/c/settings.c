#include <pebble.h>
#include <pebble-events/pebble-events.h>

#include "settings.h"
#include "global.h"
#include "weather.h"
#include "calendar.h"
#include "battery.h"

Settings settings;

static void save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void conf_inbox_received_handler(DictionaryIterator *iter, void *context) {
  bool updateWeather = false;
  
  Tuple *conf = dict_find(iter, MESSAGE_KEY_wsd);
  if(conf) {
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

  conf = dict_find(iter, MESSAGE_KEY_wf);
  if (conf) {
    settings.forecast = conf->value->int8 == 1;
  } else {
    settings.forecast = false;
  }

  conf = dict_find(iter, MESSAGE_KEY_wp);
  if (conf) {
    char old = settings.weatherProvider;
    settings.weatherProvider = conf->value->cstring[0];
    if (old != settings.weatherProvider) {
      updateWeather = true;
    }
  }

  conf = dict_find(iter, MESSAGE_KEY_wak);
  if (conf) {
    char old[20];
    strncpy(old, settings.weatherApiKey, 20);
    strncpy(settings.weatherApiKey, conf->value->cstring, 20);
    if (strcmp(old, settings.weatherApiKey)!=0) {
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

  if (persist_exists(SETTINGS_KEY)) {
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  }

}

void settings_init() {
  // Clay
  events_app_message_register_inbox_received(conf_inbox_received_handler, NULL);
  events_app_message_open();
}
