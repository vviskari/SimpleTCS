#include <pebble-events/pebble-events.h>
#include <pebble-generic-weather/pebble-generic-weather.h>
#include <pebble.h>

#include "global.h"
#include "settings.h"
#include "utils.h"
#include "weather.h"

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

static GFont weatherFont;
static GFont font24;

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
static int forecastSize = 0;

static void render_weather(GenericWeatherInfo *info) {
  if (!info) {
    return;
  }

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

  switch (info->condition) {
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
  snprintf(s_condition_text, sizeof(s_condition_text), "%c", condition);
  text_layer_set_text(s_weather_icon_layer, s_condition_text);
  text_layer_set_text_color(s_weather_icon_layer, weatherColor);
}

static void weather_callback(GenericWeatherInfo *info, GenericWeatherForecast *_forecast, int _forecastSize,
                             GenericWeatherStatus status) {
  if (status == GenericWeatherStatusAvailable) {
    generic_weather_save(WEATHER_KEY);
    if (_forecast && _forecastSize > 0) {
      generic_weather_save_forecast(WEATHER_KEY_FORECAST);

      forecast = _forecast;
      forecastSize = _forecastSize;
      params = (ForecastParams){
          .maxTemp = -1000, .minTemp = 1000, .maxValue = -1000, .minValue = 1000, .maxTime = 0, .minTime = 0};

      for (int i = 0; i < forecastSize; i++) {
        int16_t temp = settings.weatherTemp == 'C' ? forecast[i].temp_c : forecast[i].temp_f;
        time_t time = forecast[i].timestamp;

        params.maxTemp = params.maxTemp < temp ? temp : params.maxTemp;
        params.minTemp = params.minTemp > temp ? temp : params.minTemp;
        params.maxTime = params.maxTime < time ? time : params.maxTime;
        params.minTime = (params.minTime == 0 || params.minTime > time) ? time : params.minTime;
      }

      // Floor and ceil the min and max temperature
      // Set absolute minimum difference for min and max temps
      if (settings.weatherTemp == 'C') {
        params.maxValue = params.maxTemp / 5 * 5 + 5;
        params.minValue = params.minTemp / 5 * 5 - 5;
        if (params.maxValue - params.minValue <= 10) {
          params.minValue = params.minValue - 5;
        }
      } else if (settings.weatherTemp == 'F') {
        params.maxValue = params.maxTemp / 10 * 10 + 10;
        params.minValue = params.minTemp / 10 * 10 - 10;
        if (params.maxValue - params.minValue <= 20) {
          params.minValue = params.minValue - 10;
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
    if (settings.forecast) {
      generic_weather_load_forecast(WEATHER_KEY_FORECAST);
    }
  }
}

static void js_ready_handler(void *context) {
  switch (settings.weatherProvider) {
    case 'y':
      generic_weather_set_provider(GenericWeatherProviderYahooWeather);
      break;
    case 'o':
      generic_weather_set_provider(GenericWeatherProviderOpenWeatherMap);
      generic_weather_set_api_key(settings.weatherApiKey);
      break;
    case 'w':
      generic_weather_set_provider(GenericWeatherProviderWeatherUnderground);
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

  // background
  GRect rect_bounds = layer_get_bounds(layer);
  graphics_fill_rect(ctx, rect_bounds, 0, GCornerNone);

  // 140 * 48
  int chartPaddingX = 16;
  int chartPaddingY = 10;
  int timeInterval = (F_WIDTH - chartPaddingX) * 1000 / forecastSize;
  int tempIntervalK = (F_HEIGHT - chartPaddingY) * 1000 / (params.maxValue - params.minValue);

  // chart border
  graphics_draw_line(ctx, GPoint(chartPaddingX, 0), GPoint(chartPaddingX, F_HEIGHT - chartPaddingY));
  graphics_draw_line(ctx, GPoint(chartPaddingX, F_HEIGHT - chartPaddingY), GPoint(F_WIDTH, F_HEIGHT - chartPaddingY));

#if defined(PBL_COLOR)
  // temperature lines
  if (true) {
    graphics_context_set_stroke_color(ctx, GColorDarkGray);
    int16_t t = params.maxValue;
    while (t > params.minValue) {
      int16_t y = (params.maxValue - t) * tempIntervalK / 1000;
      //APP_LOG(APP_LOG_LEVEL_INFO, "TEMP line %d %d", t, y);
      graphics_draw_line(ctx, GPoint(chartPaddingX + 1, y), GPoint(F_WIDTH, y));
      t = settings.weatherTemp == 'C' ? t - 5 : t - 10;
    }
  }

  // Condition colors
  for (int i = 0; i < forecastSize; i++) {
    int16_t x = i * timeInterval / 1000 + chartPaddingX + 1;
    GColor c = GColorClear;
    switch (forecast[i].condition) {
      case GenericWeatherConditionClearSky:
      case GenericWeatherConditionScatteredClouds:
      case GenericWeatherConditionFewClouds:
        c = GColorYellow;
        break;
      case GenericWeatherConditionBrokenClouds:
      case GenericWeatherConditionMist:
      case GenericWeatherConditionWind:
        c = GColorDarkGray;
        break;
      case GenericWeatherConditionShowerRain:
      case GenericWeatherConditionRain:
      case GenericWeatherConditionSnow:
      case GenericWeatherConditionThunderstorm:
        c = GColorVividCerulean;
        break;
      default:
        c = GColorClear;
    }
    graphics_context_set_fill_color(ctx, c);
    GRect conditionRect = GRect(x, F_HEIGHT - chartPaddingY - 5, timeInterval / 1000 + 1, 5);
    graphics_fill_rect(ctx, conditionRect, 0, GCornerNone);
  }
#endif
  // Temp line graph
  GPoint start = GPoint(-1, -1);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  for (int i = 0; i < forecastSize; i++) {
    int16_t x = i * timeInterval / 1000 + (timeInterval / 2000);
    int16_t y = ((settings.weatherTemp == 'C' ? forecast[i].temp_c : forecast[i].temp_f) - params.minValue) *
                tempIntervalK / 1000;
    x = x + chartPaddingX;
    y = y + chartPaddingY;
    GPoint end = GPoint(x, F_HEIGHT - y);
    if (start.x >= 0) {
      // draw line
      graphics_draw_line(ctx, start, end);
    }
    start = end;
  }

  // Y-temps
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  static char s_temp_text[6];
  rect_bounds = GRect(-2, -4, 17, 10);
  snprintf(s_temp_text, sizeof(s_temp_text), "%d", params.maxValue);
  graphics_draw_text(ctx, s_temp_text, font, rect_bounds, GTextOverflowModeFill, GTextAlignmentRight, NULL);

  rect_bounds = GRect(-2, F_HEIGHT - 13 - chartPaddingY, 17, 10);
  snprintf(s_temp_text, sizeof(s_temp_text), "%d", params.minValue);
  graphics_draw_text(ctx, s_temp_text, font, rect_bounds, GTextOverflowModeFill, GTextAlignmentRight, NULL);

  // X-times
  font = fonts_get_system_font(FONT_KEY_GOTHIC_09);
  int idx = forecastSize / 3;
  time_t time = forecast[idx].timestamp;
  static char hour[3];
  strftime(hour, 3, "%H", localtime(&time));
  rect_bounds = GRect(idx * timeInterval / 1000 + chartPaddingX - 6, F_HEIGHT - 11, 12, 10);
  graphics_draw_text(ctx, hour, font, rect_bounds, GTextOverflowModeFill, GTextAlignmentCenter, NULL);

  idx = forecastSize * 2 / 3;
  time = forecast[idx].timestamp;
  strftime(hour, 3, "%H", localtime(&time));
  rect_bounds = GRect(idx * timeInterval / 1000 + chartPaddingX - 6, F_HEIGHT - 11, 12, 10);
  graphics_draw_text(ctx, hour, font, rect_bounds, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

void handle_weather(bool refresh) {
  if (refresh && !userIsSleeping()) {
    app_timer_register(3000, js_ready_handler, NULL);
  } else {
    GenericWeatherPeekData peek = generic_weather_peek();
    if (!peek.info) {
      app_timer_register(3000, js_ready_handler, NULL);
    } else {
      weather_callback(peek.info, peek.forecast, peek.forecastSize, GenericWeatherStatusAvailable);
    }
  }
}

void hide_weather(bool hide) {
  layer_set_hidden((Layer *)s_weather_text_layer, hide);
  layer_set_hidden((Layer *)s_weather_loc_layer, hide);
  layer_set_hidden((Layer *)s_weather_unit_layer, hide);
}

void hide_forecast(bool hide) {
  layer_set_hidden(s_weather_forecast_layer, hide);
}

void weather_load() {
  Layer *window_layer = window_get_root_layer(s_main_window);

  weatherFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHER_30));
  font24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_24));
  s_weather_icon_layer = text_layer_create(GRect(58, 85, 32, 35));
  text_layer_set_text_color(s_weather_icon_layer, GColorWhite);
  text_layer_set_background_color(s_weather_icon_layer, GColorClear);
  text_layer_set_font(s_weather_icon_layer, weatherFont);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_icon_layer));

  s_weather_text_layer = text_layer_create(GRect(87, 83, 55, 30));
  text_layer_set_text_color(s_weather_text_layer, GColorWhite);
  text_layer_set_background_color(s_weather_text_layer, GColorClear);
  text_layer_set_font(s_weather_text_layer, font24);
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

  s_weather_forecast_layer = layer_create(GRect(2, 120, F_WIDTH, F_HEIGHT));
  layer_set_update_proc(s_weather_forecast_layer, forecast_update_proc);
  hide_forecast(true);
  layer_add_child(window_layer, s_weather_forecast_layer);

  handle_weather(false);
}

void weather_unload() {
  text_layer_destroy(s_weather_icon_layer);
  text_layer_destroy(s_weather_text_layer);
  text_layer_destroy(s_weather_unit_layer);
  text_layer_destroy(s_weather_loc_layer);
  layer_destroy(s_weather_forecast_layer);
  fonts_unload_custom_font(weatherFont);
  fonts_unload_custom_font(font24);
}

void weather_init() {
  generic_weather_init();
  // generic_weather_set_location((GenericWeatherCoordinates)
  // {6016437,2492361});
  generic_weather_set_forecast(settings.forecast);
  generic_weather_load(WEATHER_KEY);
  if (settings.forecast) {
    generic_weather_load_forecast(WEATHER_KEY_FORECAST);
  }
}

void weather_deinit() {
  generic_weather_deinit();
}