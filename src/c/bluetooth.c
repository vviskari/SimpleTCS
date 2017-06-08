#include "bluetooth.h"
#include <pebble.h>
#include "global.h"
#include "utils.h"

static TextLayer *s_connection_layer;
static GBitmap *s_bluetooth_bitmap;
static BitmapLayer *s_bluetooth_bitmap_layer;
static bool lastconnected = true;

static void handle_bluetooth(bool connected) {
  Layer *window_layer = window_get_root_layer(s_main_window);
  text_layer_set_text(s_connection_layer, connected ? "online" : "OFFLINE");
  bitmap_layer_destroy(s_bluetooth_bitmap_layer);
  text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

  if (connected) {
    text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_color(s_connection_layer, GColorGreen);
    s_bluetooth_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT);
    lastconnected = true;
    if (!lastconnected && !userIsSleeping()) {
      static uint32_t const segments[] = {300, 100, 100, 100, 100};
      VibePattern pat = {
          .durations = segments, .num_segments = ARRAY_LENGTH(segments),
      };
      vibes_enqueue_custom_pattern(pat);
    }
  } else {
#if defined(PBL_BW)
    text_layer_set_text_color(s_connection_layer, GColorWhite);
    s_bluetooth_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT);
#else
    text_layer_set_text_color(s_connection_layer, GColorRed);
    s_bluetooth_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NOBT);
#endif
    lastconnected = false;
    if (!userIsSleeping()) {
      static uint32_t const segments[] = {300, 300, 300, 300, 300};
      VibePattern pat = {
          .durations = segments, .num_segments = ARRAY_LENGTH(segments),
      };
      vibes_enqueue_custom_pattern(pat);
    }
  }
  s_bluetooth_bitmap_layer = bitmap_layer_create(GRect(3, 2, 8, 13));
  bitmap_layer_set_bitmap(s_bluetooth_bitmap_layer, s_bluetooth_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bluetooth_bitmap_layer));
}

void bluetooth_load() {
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_frame(window_layer);

  s_connection_layer = text_layer_create(GRect(16, -5, bounds.size.w, 18));
  text_layer_set_text_color(s_connection_layer, GColorWhite);
  text_layer_set_background_color(s_connection_layer, GColorClear);
  text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  handle_bluetooth(connection_service_peek_pebble_app_connection());

  layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));

  connection_service_subscribe((ConnectionHandlers){.pebble_app_connection_handler = handle_bluetooth});
}

void bluetooth_unload() {
  connection_service_unsubscribe();
  text_layer_destroy(s_connection_layer);
  gbitmap_destroy(s_bluetooth_bitmap);
  bitmap_layer_destroy(s_bluetooth_bitmap_layer);
}