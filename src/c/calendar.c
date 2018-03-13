#include "calendar.h"
#include <pebble.h>
#include "global.h"
#include "settings.h"

static TextLayer *s_cal_array_layer[3][7];

static bool is_day_idx_weekend(int idx) {
  if (settings.weekStartDay == 's') {
    if (idx % 7 == 0 || idx % 7 == 6) {
      return true;
    }
  } else {
    if (idx % 7 >= 5) {
      return true;
    }
  }
  return false;
}

void drawcal() {
  static char monthdaynumber[3];
  char weekdaynumber[2];
  time_t now = time(NULL);
  // Day of the month (01-31)
  strftime(monthdaynumber, 3, "%d", localtime(&now));
  // The weekday as a number, 1-based from Monday (from ‘1’ to ‘7’). [tm_wday]
  strftime(weekdaynumber, 2, "%u", localtime(&now));

  static char week[3][7][3];

  int day_id = 0;
  int offset = (settings.weekStartDay == 's') ? 0 : 1;

  // start from beginning of last week (depending if week starts on mon or sun)
  for (int x = offset - atoi(weekdaynumber) - 7; x < 14 + offset - atoi(weekdaynumber); x++) {
    // timestamp adjusted by x days
    time_t tt = time(NULL) + x * 24 * 3600;
    struct tm *stm = localtime(&tt);
    if (day_id >= 14) {
      if (is_day_idx_weekend(day_id)) {
        text_layer_set_font(s_cal_array_layer[2][day_id - 14], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
      } else {
        text_layer_set_font(s_cal_array_layer[2][day_id - 14], fonts_get_system_font(FONT_KEY_GOTHIC_18));
      }
      strftime(week[2][day_id - 14], 3, "%d", stm);
      text_layer_set_text(s_cal_array_layer[2][day_id - 14], week[2][day_id - 14]);
    } else if (day_id >= 7) {
      if (is_day_idx_weekend(day_id)) {
        text_layer_set_font(s_cal_array_layer[1][day_id - 7], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
      } else {
        text_layer_set_font(s_cal_array_layer[1][day_id - 7], fonts_get_system_font(FONT_KEY_GOTHIC_18));
      }
      strftime(week[1][day_id - 7], 3, "%d", stm);
      text_layer_set_text(s_cal_array_layer[1][day_id - 7], week[1][day_id - 7]);

      if (strcmp(monthdaynumber, week[1][day_id - 7]) == 0) {
#if defined(PBL_COLOR)
        text_layer_set_font(s_cal_array_layer[1][day_id - 7], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
        text_layer_set_background_color(s_cal_array_layer[1][day_id - 7], GColorDarkCandyAppleRed);
#else
        text_layer_set_font(s_cal_array_layer[1][day_id - 7], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
        text_layer_set_background_color(s_cal_array_layer[1][day_id - 7], GColorWhite);
        text_layer_set_text_color(s_cal_array_layer[1][day_id - 7], GColorBlack);
#endif
      } else {
        text_layer_set_background_color(s_cal_array_layer[1][day_id - 7], GColorClear);
        text_layer_set_text_color(s_cal_array_layer[1][day_id - 7], GColorWhite);
      }
    } else {
      if (is_day_idx_weekend(day_id)) {
        text_layer_set_font(s_cal_array_layer[0][day_id], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
      } else {
        text_layer_set_font(s_cal_array_layer[0][day_id], fonts_get_system_font(FONT_KEY_GOTHIC_18));
      }
      strftime(week[0][day_id], 3, "%d", stm);
      text_layer_set_text(s_cal_array_layer[0][day_id], week[0][day_id]);
      text_layer_set_background_color(s_cal_array_layer[0][day_id], GColorBlack);
    }
    day_id++;
  }
}

void calendar_load() {
  for (int week = 2; week >= 0; week--) {
    for (int day = 0; day < 7; day++) {
      s_cal_array_layer[week][day] = text_layer_create(GRect(2 + day * 20, week * 15, 20, 21));
      text_layer_set_text_color(s_cal_array_layer[week][day], GColorWhite);
      text_layer_set_background_color(s_cal_array_layer[week][day], GColorClear);
      text_layer_set_font(s_cal_array_layer[week][day], fonts_get_system_font(FONT_KEY_GOTHIC_18));
      text_layer_set_text_alignment(s_cal_array_layer[week][day], GTextAlignmentCenter);
      text_layer_set_text(s_cal_array_layer[week][day], "00");
      layer_add_child(s_calendar_container, text_layer_get_layer(s_cal_array_layer[week][day]));
    }
  }
  drawcal();
}

void calendar_unload() {
  for (int week = 2; week >= 0; week--) {
    for (int day = 0; day < 7; day++) {
      text_layer_destroy(s_cal_array_layer[week][day]);
    }
  }
}
