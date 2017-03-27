#pragma once
#include <pebble.h>

void handle_weather(bool refresh);
void hide_weather(bool hide);
void hide_forecast(bool hide);

void weather_load();
void weather_unload();

void weather_init();
void weather_deinit();
