#include "datetime.h"
#include <pebble.h>
#include "global.h"
#include "settings.h"

static TextLayer *s_date_layer;
static TextLayer *s_time_layer;
static TextLayer *s_time_layer_shadow;
static TextLayer *s_seconds_layer;
static GFont font52;
static GFont font32;
const char df1[] = "%d.%m.%Y";
const char df2[] = "%m/%d/%Y";
const char df3[] = "%Y-%m-%d";
const char df4[] = "%d.%m.";
const char df5[] = "%m.%d.";

static char weekdayname[6][7][15] = {{"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"},
                                     {"Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag", "Sonntag"},
                                     {"lunes", "martes", "miércoles", "jueves", "viernes", "sábado", "domingo"},
                                     {"Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche"},
                                     {"Lunedì", "Martedì", "Mercoledì", "Giovedì", "Venerdì", "Sabato", "Domenica"},
                                     {"Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado", "Domingo"}};

void handle_time(struct tm *tick_time, TimeUnits units_changed) {
  char weekdaynumber[] = "0";
  char *sys_locale = setlocale(LC_ALL, "");
  static int loc = 0;

  if (strcmp("de_DE", sys_locale) == 0) {
    loc = 1;
  } else if (strcmp("es_ES", sys_locale) == 0) {
    loc = 2;
  } else if (strcmp("fr_FR", sys_locale) == 0) {
    loc = 3;
  } else if (strcmp("it_IT", sys_locale) == 0) {
    loc = 4;
  } else if (strcmp("pt_PT", sys_locale) == 0) {
    loc = 5;
  } else {
    loc = 0;
  }

  // Time
  static char s_time_text[] = "00:00";
  if (clock_is_24h_style()) {
    strftime(s_time_text, sizeof(s_time_text), "%H:%M", tick_time);
  } else {
    strftime(s_time_text, sizeof(s_time_text), "%I:%M", tick_time);
  }
  text_layer_set_text(s_time_layer, s_time_text);
  #if defined(PBL_COLOR)
  text_layer_set_text(s_time_layer_shadow, s_time_text);
  #endif

  // Time and date
  static char s_date_text[] = "XXXXXXXXXXXXXXX 12.12.1999";
  strftime(weekdaynumber, sizeof(weekdaynumber), "%u", tick_time);

  static char dmy[] = "12.12.1999";
  if (settings.dateFormat == '1') {
    strftime(dmy, sizeof(dmy), df1, tick_time);
  } else if (settings.dateFormat == '2') {
    strftime(dmy, sizeof(dmy), df2, tick_time);
  } else if (settings.dateFormat == '3') {
    strftime(dmy, sizeof(dmy), df3, tick_time);
  } else if (settings.dateFormat == '4') {
    strftime(dmy, sizeof(dmy), df4, tick_time);
  } else if (settings.dateFormat == '5') {
    strftime(dmy, sizeof(dmy), df5, tick_time);
  }
  
  int weekday_idx = atoi(weekdaynumber) - 1;
  char* weekdayname_loc = weekdayname[loc][weekday_idx];
  if (settings.translateWeekdays) {
    if (weekday_idx == 0) {
      weekdayname_loc = settings.monday;
    }
    if (weekday_idx == 1) {
      weekdayname_loc = settings.tuesday;
    }
    if (weekday_idx == 2) {
      weekdayname_loc = settings.wednesday;
    }
    if (weekday_idx == 3) {
      weekdayname_loc = settings.thursday;
    }
    if (weekday_idx == 4) {
      weekdayname_loc = settings.friday;
    }
    if (weekday_idx == 5) {
      weekdayname_loc = settings.saturday;
    }
    if (weekday_idx == 6) {
      weekdayname_loc = settings.sunday;
    }
  }
  //APP_LOG(APP_LOG_LEVEL_INFO, "Weekday LOC %s", weekdayname_loc);
  
  snprintf(s_date_text, sizeof(s_date_text), "%s %s", weekdayname_loc, dmy);
  text_layer_set_text(s_date_layer, s_date_text);
}

void handle_seconds(struct tm *tick_time) {
  if (!tick_time) {
    text_layer_set_text(s_seconds_layer, "");
  } else {
    static char s_seconds_text[] = "00";
    strftime(s_seconds_text, sizeof(s_seconds_text), "%S", tick_time);
    text_layer_set_text(s_seconds_layer, s_seconds_text);
  }
}

void hide_seconds(bool hide) { layer_set_hidden((Layer *)s_seconds_layer, hide); }

void datetime_load() {
  Layer *window_layer = window_get_root_layer(s_main_window);
  font52 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_52));
  font32 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_32));

  s_time_layer_shadow = text_layer_create(GRect(2, 30, 144, 55));
  text_layer_set_text_color(s_time_layer_shadow, GColorClear);  
  text_layer_set_background_color(s_time_layer_shadow, GColorClear);
  text_layer_set_font(s_time_layer_shadow, font52);
  text_layer_set_text_alignment(s_time_layer_shadow, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer_shadow));

  s_time_layer = text_layer_create(GRect(0, 28, 144, 55));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, font52);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_date_layer = text_layer_create(GRect(0, 16, 144, 34));
  text_layer_set_text_color(s_date_layer, GColorYellow);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  s_seconds_layer = text_layer_create(GRect(100, 80, 50, 40));
#if defined(PBL_BW)
  text_layer_set_text_color(s_seconds_layer, GColorWhite);
#else
  text_layer_set_text_color(s_seconds_layer, GColorOrange);
#endif
  text_layer_set_background_color(s_seconds_layer, GColorClear);
  text_layer_set_font(s_seconds_layer, font32);
  layer_add_child(window_layer, text_layer_get_layer(s_seconds_layer));

  hide_seconds(true);
}

void datetime_unload() {
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_time_layer_shadow);
  text_layer_destroy(s_seconds_layer);
  fonts_unload_custom_font(font52);
  fonts_unload_custom_font(font32);
}

void set_time_shadow(int shadow) {
#if defined(PBL_COLOR)
  text_layer_set_text_color(s_time_layer_shadow, GColorLightGray);

  if (shadow == SHADOW_CLEAR) {
    text_layer_set_text_color(s_time_layer_shadow, GColorChromeYellow);    
  } else if (shadow == SHADOW_RAIN) {
    text_layer_set_text_color(s_time_layer_shadow, GColorVividCerulean);
  }
#endif
}