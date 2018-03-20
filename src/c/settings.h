#pragma once
#include <pebble.h>

typedef struct {
  char weekStartDay;
  char weatherTemp;
  char weatherProvider;
  char weatherApiKey[20];
  bool forecast;
  char viewMode;
  char dateFormat;
  bool translateWeekdays;
  char monday[20];
  char tuesday[20];
  char wednesday[20];
  char thursday[20];
  char friday[20];
  char saturday[20];
  char sunday[20];
  bool tempGrid;
  char forecastTimeInterval;
  bool stickyLocation;
  bool animationEnabled;
} Settings;

extern Settings settings;

void load_settings();
void settings_init();
void settings_deinit();
