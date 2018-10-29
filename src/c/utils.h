#pragma once
#include <pebble.h>

bool userIsSleeping();
void set_show_forecast(bool show);
int get_time_hour(time_t time);