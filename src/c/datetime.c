#include <pebble.h>
#include "datetime.h"
#include "global.h"

static TextLayer *s_date_layer;
static TextLayer *s_time_layer;
static TextLayer *s_seconds_layer;
static GFont font52;
static GFont font32;

static char weekdayname[6][7][15] = {
  {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"},
  {"Montag","Dienstag","Mittwoch","Donnerstag","Freitag","Samstag","Sonntag"},
  {"lunes","martes","miércoles","jueves","viernes","sábado","domingo"},
  {"Lundi","Mardi","Mercredi","Jeudi","Vendredi","Samedi","Dimanche"},
  {"Lunedì","Martedì","Mercoledì","Giovedì","Venerdì","Sabato","Domenica"},
  {"Segunda","Terça","Quarta","Quinta","Sexta","Sábado","Domingo"}
};

void handle_time(struct tm* tick_time, TimeUnits units_changed) {
  char weekdaynumber[] = "0";
  char *sys_locale = setlocale(LC_ALL, "");
  static int loc = 0;

  if (strcmp("de_DE", sys_locale) == 0) {
    loc=1;
  } else if (strcmp("es_ES", sys_locale) == 0) {
    loc=2;
  } else if (strcmp("fr_FR", sys_locale) == 0) {
    loc=3;
  } else if (strcmp("it_IT", sys_locale) == 0) {
    loc=4;
  } else if (strcmp("pt_PT", sys_locale) == 0) {
    loc=5;
  } else {
    loc=0;
  }

  // Time
  static char s_time_text[] = "00:00";
  if (clock_is_24h_style()) {
    strftime(s_time_text, sizeof(s_time_text), "%H:%M", tick_time);    
  } else {
    strftime(s_time_text, sizeof(s_time_text), "%I:%M", tick_time);
  }
  text_layer_set_text(s_time_layer, s_time_text);

  // Time and date
  static char s_date_text[] = "XXXXXXXXXXXXX, 12.12.1999";
  strftime(weekdaynumber, sizeof(weekdaynumber), "%u", tick_time);

  static char dmy[] = "XXXXXXXXXXXXX, 12.12.1999";
  strftime(dmy, sizeof(dmy), "%d.%m.%Y", tick_time);

  snprintf(s_date_text, sizeof(s_date_text), "%s, %s", weekdayname[loc][atoi(weekdaynumber)-1], dmy);
  text_layer_set_text(s_date_layer, s_date_text);
}

void handle_seconds(struct tm* tick_time) {
  if (!tick_time) {
    text_layer_set_text(s_seconds_layer, "");
  } else {
    static char s_seconds_text[] = "00";
    strftime(s_seconds_text, sizeof(s_seconds_text), "%S", tick_time);
    text_layer_set_text(s_seconds_layer, s_seconds_text);
  }
}

void hide_seconds(bool hide) {
  layer_set_hidden((Layer *) s_seconds_layer, hide);
}

void datetime_load() {
  Layer *window_layer = window_get_root_layer(s_main_window);
  font52 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_52));
  font32 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_32));
  
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
  text_layer_destroy(s_seconds_layer);
  fonts_unload_custom_font(font52);
  fonts_unload_custom_font(font32);
}
