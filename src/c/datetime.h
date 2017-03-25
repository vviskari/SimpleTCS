#pragma once
#include <pebble.h>

void handle_time(struct tm* tick_time, TimeUnits units_changed);
void handle_seconds(struct tm* tick_time);
void datetime_load();
void datetime_unload();
void hide_seconds(bool hide);
