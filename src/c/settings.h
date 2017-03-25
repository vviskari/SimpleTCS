#pragma once

typedef struct {
  char weekStartDay;
  char weatherTemp;
  char weatherProvider;
  char weatherApiKey[20];
} Settings;

extern Settings settings;

void load_settings();
void settings_init();
