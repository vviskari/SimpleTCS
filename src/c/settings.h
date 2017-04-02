#pragma once
#include <pebble.h>

typedef struct {
  char weekStartDay;
  char weatherTemp;
  char weatherProvider;
  char weatherApiKey[20];
  bool forecast;
} Settings;

extern Settings settings;

void load_settings();
void settings_init();
void settings_deinit();
