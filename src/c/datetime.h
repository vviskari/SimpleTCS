#pragma once
#include <pebble.h>

#define SHADOW_CLEAR 1
#define SHADOW_CLOUD 2
#define SHADOW_RAIN 3

void handle_time(struct tm* tick_time, TimeUnits units_changed);
void handle_seconds(struct tm* tick_time);
void datetime_load();
void datetime_unload();
void hide_seconds(bool hide);
void set_time_shadow(int shadow);
