#include "battery.h"
#include <pebble.h>
#include "global.h"
#include "settings.h"

static TextLayer *s_battery_layer;
static TextLayer *s_box;
static TextLayer *s_bat_cal_bg_layer[3];
static TextLayer *s_bat_cal_bat_layer[3];
static GBitmap *s_battery_bitmap;
static BitmapLayer *s_battery_bitmap_layer;

static const int weekSeconds = 60 * 60 * 24 * 7;

static void estimate_battery(BatteryChargeState charge_state) {
  time_t now = time(NULL);

  // Store current charging state
  bool lastCharging = false;
  if (persist_exists(BATT_CHARGING_KEY)) {
    lastCharging = persist_read_bool(BATT_CHARGING_KEY);
  }
  persist_delete(BATT_CHARGING_KEY);
  persist_write_bool(BATT_CHARGING_KEY, charge_state.is_plugged);

  // skip everything, if charging
  if (charge_state.is_plugged) {
    // APP_LOG(APP_LOG_LEVEL_INFO, "Charging, skip");
    return;
  }

  // default history
  BatteryHistory history = (BatteryHistory){
      .timestamp = 0,                         // marks null history if time is 0
      .charge = charge_state.charge_percent,  // charge when last disconnected
      .last_estimated = 0,                    // charge when last estimated battery life
      .slope = -694                           // Initial estimate for full battery is 6d
  };

  if (persist_exists(BATT_HISTORY_KEY)) {
    persist_read_data(BATT_HISTORY_KEY, &history, sizeof(BatteryHistory));
    // APP_LOG(APP_LOG_LEVEL_INFO, "Loaded history, charge=%d, slope=%d",
    // history.charge, history.slope);
  }

  if (lastCharging && !charge_state.is_plugged) {
    // APP_LOG(APP_LOG_LEVEL_INFO, "Charger disconnected. Store state to
    // history");
    history.timestamp = now;
    history.charge = charge_state.charge_percent;
  }

  if (history.timestamp == 0) {
    // APP_LOG(APP_LOG_LEVEL_INFO, "No old history. return");
    return;
  }

  int historyTimeHours = history.timestamp / 60 / 60;
  int historyChargePercent = history.charge * 1000;

  if (history.last_estimated != charge_state.charge_percent && charge_state.charge_percent < 90) {
    // new estimation
    // skip battery levels 100 and 90, they are not reliable
    int currentTimeHours = (int)now / 60 / 60;
    int currentChargePercent = charge_state.charge_percent * 1000;

    // calculate slope for current charge against history charge (weighted)
    history.slope =
        (2 * history.slope + (currentChargePercent - historyChargePercent) / (currentTimeHours - historyTimeHours)) / 3;
    history.last_estimated = charge_state.charge_percent;
  }

  // estimated time when battery is 0
  int estimatedBatteryLife = (-historyChargePercent + history.slope * historyTimeHours) / history.slope * 60 * 60;
  // APP_LOG(APP_LOG_LEVEL_INFO, "Estimated battery life until %d",
  // estimatedBatteryLife);
  persist_delete(BATT_HISTORY_KEY);
  persist_write_data(BATT_HISTORY_KEY, &history, sizeof(BatteryHistory));

  ///////////////
  // start render

  // current week day 0-6
  char weekdaynumber[2];
  strftime(weekdaynumber, 2, "%u", localtime(&now));
  struct tm *stm = localtime(&now);
  int offset = (settings.weekStartDay == 's') ? 0 : 1;
  int weekStart =
      now - (atoi(weekdaynumber) - offset) * 60 * 60 * 24 - stm->tm_hour * 60 * 60 - stm->tm_min * 60 - stm->tm_sec;

  Layer *window_layer = window_get_root_layer(s_main_window);
  bool is_bat_cal_hidden = layer_get_hidden((Layer *)s_bat_cal_bat_layer[1]);
  text_layer_destroy(s_bat_cal_bat_layer[0]);
  text_layer_destroy(s_bat_cal_bat_layer[1]);
  text_layer_destroy(s_bat_cal_bat_layer[2]);

  // last week
  if (history.timestamp < weekStart) {
    // history starts last week
    int start = 140 - (weekStart - history.timestamp) * 140 / weekSeconds;
    if (start < 0) {
      start = 0;
    }
    int width = 140 - start;
    // APP_LOG(APP_LOG_LEVEL_INFO, "Last week %d, %d", start, width);
    s_bat_cal_bat_layer[0] = text_layer_create(GRect(2 + start, 134, width, 2));
    text_layer_set_background_color(s_bat_cal_bat_layer[0], GColorSpringBud);
    layer_set_hidden((Layer *)s_bat_cal_bat_layer[0], is_bat_cal_hidden);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bat_layer[0]));
  } else {
    // no history last week
    s_bat_cal_bat_layer[0] = text_layer_create(GRect(2, 134, 140, 2));
    text_layer_set_background_color(s_bat_cal_bat_layer[0], GColorBlack);
    layer_set_hidden((Layer *)s_bat_cal_bat_layer[0], is_bat_cal_hidden);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bat_layer[0]));
  }

  // this week
  if (true) {
    int start = 0;
    if (history.timestamp > weekStart) {
      // history starts this week
      start = (history.timestamp - weekStart) * 140 / weekSeconds;
    }
    int width = 140 - start;
    if (estimatedBatteryLife - weekStart < weekSeconds) {
      // estimation ends this week
      width = (estimatedBatteryLife - weekStart) * 140 / weekSeconds - start;
    }
    // APP_LOG(APP_LOG_LEVEL_INFO, "This week %d, %d", start, width);
    s_bat_cal_bat_layer[1] = text_layer_create(GRect(2 + start, 134 + 15, width, 2));
    text_layer_set_background_color(s_bat_cal_bat_layer[1], GColorSpringBud);
    layer_set_hidden((Layer *)s_bat_cal_bat_layer[1], is_bat_cal_hidden);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bat_layer[1]));
  }

  // next week
  if (estimatedBatteryLife > weekStart + weekSeconds) {
    // estimation ends next week
    int width = (estimatedBatteryLife - weekStart - weekSeconds) * 140 / weekSeconds;
    if (width > 140) {
      width = 140;
    }
    // APP_LOG(APP_LOG_LEVEL_INFO, "Next week %d", width);

    s_bat_cal_bat_layer[2] = text_layer_create(GRect(2, 134 + 30, width, 2));
    text_layer_set_background_color(s_bat_cal_bat_layer[2], GColorSpringBud);
    layer_set_hidden((Layer *)s_bat_cal_bat_layer[2], is_bat_cal_hidden);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bat_layer[2]));
  } else {
    // black next week
    s_bat_cal_bat_layer[2] = text_layer_create(GRect(2, 134 + 30, 140, 2));
    text_layer_set_background_color(s_bat_cal_bat_layer[2], GColorBlack);
    layer_set_hidden((Layer *)s_bat_cal_bat_layer[2], is_bat_cal_hidden);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bat_layer[2]));
  }
}

static void render_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  Layer *window_layer = window_get_root_layer(s_main_window);
  text_layer_destroy(s_box);
  int width = 22 * charge_state.charge_percent / 100;
  s_box = text_layer_create(GRect(106 - width, 4, width, 9));
  layer_add_child(window_layer, text_layer_get_layer(s_box));

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "%d%%+", charge_state.charge_percent);
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
    text_layer_set_text_color(s_battery_layer, GColorRed);
    text_layer_set_background_color(s_box, GColorRed);
    if (charge_state.charge_percent > 20) {
      text_layer_set_text_color(s_battery_layer, GColorYellow);
      text_layer_set_background_color(s_box, GColorYellow);
    }
    if (charge_state.charge_percent > 40) {
      text_layer_set_text_color(s_battery_layer, GColorGreen);
      text_layer_set_background_color(s_box, GColorGreen);
    }
  }
#if defined(PBL_BW)
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_background_color(s_box, GColorWhite);
#endif
  text_layer_set_text(s_battery_layer, battery_text);
  estimate_battery(charge_state);
}

void handle_battery() { render_battery(battery_state_service_peek()); }

void hide_battery_estimate(bool hide) {
  for (int week = 2; week >= 0; week--) {
    layer_set_hidden((Layer *)s_bat_cal_bg_layer[week], hide);
    layer_set_hidden((Layer *)s_bat_cal_bat_layer[week], hide);
  }
}

void battery_load() {
  Layer *window_layer = window_get_root_layer(s_main_window);

  s_battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BAT2);
  s_battery_bitmap_layer = bitmap_layer_create(GRect(80, 2, 28, 13));
  bitmap_layer_set_bitmap(s_battery_bitmap_layer, s_battery_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_battery_bitmap_layer));

  s_battery_layer = text_layer_create(GRect(109, -4, 36, 18));
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));

  s_box = text_layer_create(GRect(0, 0, 1, 1));
  text_layer_set_background_color(s_box, GColorRed);
  layer_add_child(window_layer, text_layer_get_layer(s_box));

  for (int week = 2; week >= 0; week--) {
    // Battery estimation bar
    s_bat_cal_bg_layer[week] = text_layer_create(GRect(2, 134 + week * 15, 140, 2));
    text_layer_set_background_color(s_bat_cal_bg_layer[week], GColorBlack);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bg_layer[week]));

    s_bat_cal_bat_layer[week] = text_layer_create(GRect(2, 134 + week * 15, 140, 2));
    text_layer_set_background_color(s_bat_cal_bat_layer[week], GColorBlack);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bat_layer[week]));
  }

  battery_state_service_subscribe(render_battery);
  handle_battery();
}

void battery_unload() {
  battery_state_service_unsubscribe();

  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_box);
  gbitmap_destroy(s_battery_bitmap);
  bitmap_layer_destroy(s_battery_bitmap_layer);

  for (int week = 2; week >= 0; week--) {
    text_layer_destroy(s_bat_cal_bg_layer[week]);
    text_layer_destroy(s_bat_cal_bat_layer[week]);
  }
}
