#include <pebble.h>
#include "steps.h"
#include "global.h"
#include "utils.h"

#if defined(PBL_HEALTH)
static BitmapLayer *s_shoe_bitmap_layer;
static GBitmap *s_shoe_bitmap;
static TextLayer *s_steps_layer;
#endif

void handle_steps() {
  #if defined(PBL_HEALTH)
  static char s_steps_text[] = "N/A  ";

  if(!userIsSleeping()) {
    HealthMetric metric = HealthMetricStepCount;
    const HealthServiceTimeScope scope = HealthServiceTimeScopeDailyWeekdayOrWeekend;

    time_t start = time_start_of_today();
    time_t end = time(NULL);

    // Check the metric has data available for today
    HealthServiceAccessibilityMask stepsAvailable = health_service_metric_accessible(metric, start, end);
    HealthServiceAccessibilityMask averageAvailable = health_service_metric_averaged_accessible(metric, start, end, scope);

    if(stepsAvailable & averageAvailable & HealthServiceAccessibilityMaskAvailable) {
      int stepsToday = (int)health_service_sum_today(metric);
      //APP_LOG(APP_LOG_LEVEL_INFO, "Steps data available: %d", stepsToday);

      snprintf(s_steps_text, sizeof(s_steps_text), "%d", stepsToday);    

      text_layer_set_text_color(s_steps_layer, GColorWhite);
      #if defined(PBL_COLOR)
      int average = (int) health_service_sum_averaged(metric, start, end, scope);
      //APP_LOG(APP_LOG_LEVEL_INFO, "Average step count: %d steps", (int)average);
      int diff = 100*stepsToday/average;
      if (diff < 60) {
        text_layer_set_text_color(s_steps_layer, GColorOrange);
      } else if (diff > 95) {
        text_layer_set_text_color(s_steps_layer, GColorGreen);
      }
      #endif
    }
  }
  text_layer_set_text(s_steps_layer, s_steps_text);
  #endif
}

void steps_load(){
  #if defined(PBL_HEALTH)
  Layer *window_layer = window_get_root_layer(s_main_window);
  s_steps_layer = text_layer_create(GRect(0, 96, 50, 18));
  text_layer_set_text_color(s_steps_layer, GColorWhite);
  text_layer_set_background_color(s_steps_layer, GColorClear);
  text_layer_set_text_alignment(s_steps_layer, GTextAlignmentCenter);
  text_layer_set_font(s_steps_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(s_steps_layer, "");
  layer_add_child(window_layer, text_layer_get_layer(s_steps_layer));

  s_shoe_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHOE);
  s_shoe_bitmap_layer = bitmap_layer_create(GRect(12, 88, 24, 12));
  bitmap_layer_set_bitmap(s_shoe_bitmap_layer, s_shoe_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_shoe_bitmap_layer));  
  #endif

  handle_steps();
}

void steps_unload() {
  #if defined(PBL_HEALTH)
  text_layer_destroy(s_steps_layer);
  gbitmap_destroy(s_shoe_bitmap);
  bitmap_layer_destroy(s_shoe_bitmap_layer);
  #endif
}
