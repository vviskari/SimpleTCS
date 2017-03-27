#include <pebble.h>
#include <pebble-generic-weather/pebble-generic-weather.h>
#include <pebble-events/pebble-events.h>

#include "weather.h"
#include "utils.h"
#include "global.h"
#include "settings.h"

static TextLayer *s_weather_text_layer;
static TextLayer *s_weather_icon_layer;
static TextLayer *s_weather_unit_layer;
static TextLayer *s_weather_loc_layer;
static Layer *s_weather_forecast_layer;

static const char WEATHER_CLEAR = 'B';
static const char WEATHER_CLEAR_NIGHT = 'C';
static const char WEATHER_PART_CLOUD = 'H';
static const char WEATHER_PART_CLOUD_NIGHT = 'I';
static const char WEATHER_FOG = 'M';
static const char WEATHER_CLOUD = 'N';
static const char WEATHER_STORM = 'P';
static const char WEATHER_LIGHT_RAIN = 'Q';
static const char WEATHER_RAIN = 'R';
static const char WEATHER_SNOW = 'W';
static const char WEATHER_WIND = 'F';
static const char WEATHER_UNKNOWN = ')';

#define F_HEIGHT 48
#define F_WIDTH 140
typedef struct {
  int16_t maxTemp;
  int16_t minTemp;
  int16_t maxValue;
  int16_t minValue;
  time_t maxTime;
  time_t minTime;  
} ForecastParams;

static GenericWeatherForecast *forecast;
static ForecastParams params;
static int forecastSize=0;

static void render_weather(GenericWeatherInfo *info) {
  if (!info) {
    return;
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Weather, %s, %s, %d", info->name, info->description, info->temp_c);
  static char s_temp_text[6];

  if (settings.weatherTemp == 'C') {
    snprintf(s_temp_text, sizeof(s_temp_text), "%d˚", info->temp_c);      
  } else {
    snprintf(s_temp_text, sizeof(s_temp_text), "%d˚", info->temp_f);
  }
  text_layer_set_text(s_weather_text_layer, s_temp_text);

  static char s_unit_text[2];
  snprintf(s_unit_text, sizeof(s_unit_text), "%c", settings.weatherTemp);
  text_layer_set_text(s_weather_unit_layer, s_unit_text);

  text_layer_set_text(s_weather_loc_layer, info->name);

  char condition = WEATHER_UNKNOWN;
  GColor weatherColor = GColorWhite;

  switch(info->condition) {
    case GenericWeatherConditionClearSky:
      condition = info->day ? WEATHER_CLEAR : WEATHER_CLEAR_NIGHT;
      weatherColor = GColorYellow;
      break;
    case GenericWeatherConditionScatteredClouds:
    case GenericWeatherConditionFewClouds:
      condition = info->day ? WEATHER_PART_CLOUD : WEATHER_PART_CLOUD_NIGHT;
      weatherColor = GColorYellow;
      break;
    case GenericWeatherConditionBrokenClouds:
      condition = WEATHER_CLOUD;
      weatherColor = GColorLightGray;
      break;
    case GenericWeatherConditionRain:
      condition = WEATHER_RAIN;
      weatherColor = GColorLightGray;
      break;
    case GenericWeatherConditionShowerRain:
      condition = WEATHER_LIGHT_RAIN;
      break;
    case GenericWeatherConditionThunderstorm:
      condition = WEATHER_STORM;
      break;
    case GenericWeatherConditionSnow:
      condition = WEATHER_SNOW;
      weatherColor = GColorCyan;
      break;
    case GenericWeatherConditionMist:
      condition = WEATHER_FOG;
      break;  
    case GenericWeatherConditionWind:
      condition = WEATHER_WIND;
      break;
    default:
      condition = WEATHER_UNKNOWN;
  }

  static char s_condition_text[2];
  snprintf(s_condition_text, sizeof(s_condition_text), "%c",  condition);  
  text_layer_set_text(s_weather_icon_layer, s_condition_text);
  text_layer_set_text_color(s_weather_icon_layer, weatherColor);
}

static void weather_callback(GenericWeatherInfo *info, 
                             GenericWeatherForecast *_forecast,
                             int _forecastSize, 
                             GenericWeatherStatus status) {
  if (status == GenericWeatherStatusAvailable) {
    generic_weather_save(WEATHER_KEY);
    if (_forecast && _forecastSize > 0) {
      forecast = _forecast;
      forecastSize = _forecastSize;
      
      params = (ForecastParams) {
        .maxTemp  = -1000,
        .minTemp  = 1000,
        .maxValue = -1000,
        .minValue = 1000,
        .maxTime  = 0,
        .minTime  = 0
      };
      
      // find params
      for (int i = 0; i<forecastSize; i++) {
        int16_t temp = 0;
        if (settings.weatherTemp == 'C') {
          temp = forecast[i].temp_c;
        } else if (settings.weatherTemp == 'F') {
          temp = forecast[i].temp_f;
        }
        if (params.maxTemp < temp) {
          params.maxTemp = temp;
        }
        if (params.minTemp > temp) {
          params.minTemp = temp;
        }

        time_t time = forecast[i].timestamp;
        if (params.maxTime == 0) {
          params.maxTime = time;
        }
        if (params.minTime == 0) {
          params.minTime = time;
        }
        if (params.maxTime < time) {
          params.maxTime = time;
        }
        if (params.minTime > time) {
          params.minTime = time;
        }
      }
    } else {
      forecast = NULL;
      forecastSize = 0;
    }
    render_weather(info);
  } else {
    // load old state
    generic_weather_load(WEATHER_KEY);
  }
}

static void js_ready_handler(void *context) {
  switch(settings.weatherProvider) {
    case 'y':
      generic_weather_set_provider(GenericWeatherProviderYahooWeather);
      break;
    case 'o':
      generic_weather_set_provider(GenericWeatherProviderOpenWeatherMap);
      generic_weather_set_api_key(settings.weatherApiKey);
      break;
    case 'w':
      generic_weather_set_provider(GenericWeatherProviderWeatherUnderground );
      generic_weather_set_api_key(settings.weatherApiKey);
      break;
    case 'f':
      generic_weather_set_provider(GenericWeatherProviderForecastIo);
      generic_weather_set_api_key(settings.weatherApiKey);
      break;
    default:
      generic_weather_set_provider(GenericWeatherProviderUnknown);
  }
  generic_weather_set_forecast(settings.forecast);
  generic_weather_fetch(weather_callback);
}

// Render forecast chart
static void forecast_update_proc(Layer *layer, GContext *ctx) {
  if (!forecast || forecastSize == 0) {
    return;
  }
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_text_color(ctx, GColorWhite);

  GRect rect_bounds = layer_get_bounds(layer);
  graphics_fill_rect(ctx, rect_bounds, 0, GCornerNone);

  // 140 * 48
  int timeInterval = F_WIDTH/forecastSize;
  int tempIntervalK = F_HEIGHT*1000/(params.maxTemp-params.minTemp);

  GPoint start = {
    .x=-1,
    .y=-1
  };
  for (int i = 0; i<forecastSize; i++) {
    int16_t x = (i+1)*timeInterval;
    int16_t y = ((settings.weatherTemp == 'C' ? forecast[i].temp_c : forecast[i].temp_f) - params.minTemp) * tempIntervalK / 1000;
    GPoint end = { .x = x, .y = F_HEIGHT-y };
    if (start.x >= 0) {
      // draw line
      graphics_draw_line(ctx, start, end);
    }
    start = end;
  }
}

void handle_weather(bool refresh) {
  if (refresh && !userIsSleeping()) {
    app_timer_register(3000, js_ready_handler, NULL);
  } else {
    GenericWeatherInfo *info = generic_weather_peek();
    if (!info) {
      app_timer_register(3000, js_ready_handler, NULL);
    } else {
      render_weather(info);
    }
  }
}

void hide_weather(bool hide) {
  layer_set_hidden((Layer*) s_weather_text_layer, hide);
  layer_set_hidden((Layer*) s_weather_loc_layer, hide);
  layer_set_hidden((Layer*) s_weather_unit_layer, hide);
}

void hide_forecast(bool hide) {
  layer_set_hidden(s_weather_forecast_layer, hide);  
}

void weather_load(){
  Layer *window_layer = window_get_root_layer(s_main_window);

  s_weather_icon_layer = text_layer_create(GRect(58, 85, 32, 35));
  text_layer_set_text_color(s_weather_icon_layer, GColorWhite);
  text_layer_set_background_color(s_weather_icon_layer, GColorClear);
  text_layer_set_font(s_weather_icon_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHER_30)));
  layer_add_child(window_layer, text_layer_get_layer(s_weather_icon_layer));

  s_weather_text_layer = text_layer_create(GRect(87, 83, 55, 30));
  text_layer_set_text_color(s_weather_text_layer, GColorWhite);
  text_layer_set_background_color(s_weather_text_layer, GColorClear);
  text_layer_set_font(s_weather_text_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_24)));
  text_layer_set_text_alignment(s_weather_text_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_text_layer));

  s_weather_unit_layer = text_layer_create(GRect(124, 93, 15, 15));
  text_layer_set_text_color(s_weather_unit_layer, GColorWhite);
  text_layer_set_background_color(s_weather_unit_layer, GColorClear);
  text_layer_set_font(s_weather_unit_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text(s_weather_unit_layer, "C");
  text_layer_set_text_alignment(s_weather_unit_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_unit_layer));

  s_weather_loc_layer = text_layer_create(GRect(80, 108, 64, 10));
  text_layer_set_text_color(s_weather_loc_layer, GColorWhite);
  text_layer_set_background_color(s_weather_loc_layer, GColorClear);
  text_layer_set_font(s_weather_loc_layer, fonts_get_system_font(FONT_KEY_GOTHIC_09));
  text_layer_set_text_alignment(s_weather_loc_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_loc_layer));

  s_weather_forecast_layer = layer_create(GRect(2, 119, F_WIDTH, F_HEIGHT));
  layer_set_update_proc(s_weather_forecast_layer, forecast_update_proc);
  hide_forecast(true);
  layer_add_child(window_layer, s_weather_forecast_layer);

  handle_weather(false);
}

void weather_unload() {
  text_layer_destroy(s_weather_loc_layer);
  text_layer_destroy(s_weather_text_layer);
  text_layer_destroy(s_weather_unit_layer);
  text_layer_destroy(s_weather_icon_layer);
  layer_destroy(s_weather_forecast_layer);
}

void weather_init() {
  generic_weather_init();
  generic_weather_set_location((GenericWeatherCoordinates) {6047394,2509040});
  generic_weather_load(WEATHER_KEY);
}

void weather_deinit() {
  generic_weather_deinit();
}