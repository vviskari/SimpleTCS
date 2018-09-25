#include <pebble-events/pebble-events.h>
#include <pebble-generic-weather/pebble-generic-weather.h>
#include <pebble.h>

#include "datetime.h"
#include "global.h"
#include "settings.h"
#include "utils.h"
#include "weather.h"

#define WEATHER_CLEAR 'B'
#define WEATHER_CLEAR_NIGHT 'C'
#define WEATHER_PART_CLOUD 'H'
#define WEATHER_PART_CLOUD_NIGHT 'I'
#define WEATHER_FOG 'M'
#define WEATHER_CLOUD 'N'
#define WEATHER_STORM 'P'
#define WEATHER_LIGHT_RAIN 'Q'
#define WEATHER_RAIN 'R'
#define WEATHER_SNOW 'W'
#define WEATHER_WIND 'F'
#define WEATHER_UNKNOWN ')'

static TextLayer *s_weather_text_layer;
static TextLayer *s_weather_icon_layer;
static TextLayer *s_weather_unit_layer;
static TextLayer *s_weather_loc_layer;
static GFont weatherFont;
static GFont font24;

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
    APP_LOG(APP_LOG_LEVEL_INFO, "render_weather no info");
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
      set_time_shadow(SHADOW_CLEAR);
      break;
    case GenericWeatherConditionScatteredClouds:
    case GenericWeatherConditionFewClouds:
      condition = info->day ? WEATHER_PART_CLOUD : WEATHER_PART_CLOUD_NIGHT;
      weatherColor = GColorYellow;
      set_time_shadow(SHADOW_CLEAR);
      break;
    case GenericWeatherConditionBrokenClouds:
      condition = WEATHER_CLOUD;
      weatherColor = GColorLightGray;
      set_time_shadow(SHADOW_CLOUD);
      break;
    case GenericWeatherConditionMist:
      condition = WEATHER_FOG;
      weatherColor = GColorLightGray;
      set_time_shadow(SHADOW_CLOUD);
      break;
    case GenericWeatherConditionWind:
      condition = WEATHER_WIND;
      weatherColor = GColorLightGray;
      set_time_shadow(SHADOW_CLOUD);
      break;
    case GenericWeatherConditionRain:
      condition = WEATHER_RAIN;
      weatherColor = GColorVividCerulean;
      set_time_shadow(SHADOW_RAIN);
      break;
    case GenericWeatherConditionShowerRain:
      condition = WEATHER_LIGHT_RAIN;
      weatherColor = GColorVividCerulean;
      set_time_shadow(SHADOW_RAIN);
      break;
    case GenericWeatherConditionThunderstorm:
      condition = WEATHER_STORM;
      weatherColor = GColorVividCerulean;
      set_time_shadow(SHADOW_RAIN);
      break;
    case GenericWeatherConditionSnow:
      condition = WEATHER_SNOW;
      weatherColor = GColorVividCerulean;
      set_time_shadow(SHADOW_RAIN);
      break;
    default:
      condition = WEATHER_UNKNOWN;
      set_time_shadow(SHADOW_CLOUD);
  }

  static char s_condition_text[2];
  snprintf(s_condition_text, sizeof(s_condition_text), "%c", condition);
  text_layer_set_text(s_weather_icon_layer, s_condition_text);
  text_layer_set_text_color(s_weather_icon_layer, weatherColor);
}

static void weather_callback(GenericWeatherInfo *info, GenericWeatherForecast *_forecast, int _forecastSize,
                             GenericWeatherStatus status) {
  if (status != GenericWeatherStatusAvailable) {
    // load old state
    generic_weather_load(WEATHER_KEY);
    if (settings.weatherProvider == 'w' || settings.weatherProvider == 'o') {
      APP_LOG(APP_LOG_LEVEL_INFO, "weather not available. load old forecast");
      generic_weather_load_forecast(WEATHER_KEY_FORECAST);
    }
    return;
  }

  generic_weather_save(WEATHER_KEY);
  render_weather(info);

  if (settings.weatherProvider != 'w' && settings.weatherProvider != 'o') {
    return;
  }

  if (_forecastSize == 0) {
    // weather was available but no forecast received for some reason. load old one
    generic_weather_load_forecast(WEATHER_KEY_FORECAST);
  } else {
    generic_weather_save_forecast(WEATHER_KEY_FORECAST);
  }

  GenericWeatherPeekData peek = generic_weather_peek();
  if (!peek.forecast || peek.forecastSize == 0) {
    return;
  }
  forecast = peek.forecast;
  forecastSize = peek.forecastSize;
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
}

static void js_ready_handler(void *context) {
  generic_weather_set_location(GENERIC_WEATHER_GPS_LOCATION);

  GenericWeatherPeekData peek = generic_weather_peek();
  if (!!peek.info && settings.stickyLocation) {
    if (peek.info->latitude != (int32_t)0xFFFFFFFF && peek.info->longitude != (int32_t)0xFFFFFFFF &&
        peek.info->latitude != 0 && peek.info->longitude != 0) {
      generic_weather_set_location((GenericWeatherCoordinates){peek.info->latitude, peek.info->longitude});
    }
  }
  generic_weather_set_forecast(false);

  switch (settings.weatherProvider) {
    case 'y':
      generic_weather_set_provider(GenericWeatherProviderYahooWeather);
      break;
    case 'o':
      generic_weather_set_provider(GenericWeatherProviderOpenWeatherMap);
      generic_weather_set_api_key(settings.weatherApiKey);
      generic_weather_set_forecast(true);
      break;
    case 'w':
      generic_weather_set_provider(GenericWeatherProviderWeatherUnderground);
      generic_weather_set_api_key(settings.weatherApiKey);
      generic_weather_set_forecast(true);
      break;
    case 'f':
      generic_weather_set_provider(GenericWeatherProviderForecastIo);
      generic_weather_set_api_key(settings.weatherApiKey);
      break;
    default:
      generic_weather_set_provider(GenericWeatherProviderUnknown);
  }
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

  // temperature lines
  if (settings.tempGrid) {
#if defined(PBL_COLOR)
    graphics_context_set_stroke_color(ctx, GColorDarkGray);
#else
    graphics_context_set_stroke_color(ctx, GColorWhite);
#endif
    int16_t t = params.maxValue;
    while (t > params.minValue) {
      int16_t y = (params.maxValue - t) * tempIntervalK / 1000;
      // APP_LOG(APP_LOG_LEVEL_INFO, "TEMP line %d %d", t, y);
      graphics_draw_line(ctx, GPoint(chartPaddingX + 1, y), GPoint(F_WIDTH, y));

      int16_t drop = settings.weatherTemp == 'C' ? 5 : 10;
#if defined(PBL_BW)
      // avoid too many lines on BW
      int16_t minmax = params.maxValue - params.minValue;
      if (minmax / drop > 3) {
        drop = drop * 2;
      }
#endif
      t = t - drop;
    }
  }

// time markers
#if defined(PBL_COLOR)
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
#else
  graphics_context_set_stroke_color(ctx, GColorWhite);
#endif
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_09);
  rect_bounds = GRect(-2, -4, 17, 10);

  for (int i = 1; i < forecastSize; i++) {
    time_t time = forecast[i].timestamp;
    // marker string
    char s_hour[5];
    strftime(s_hour, 3, "%H", localtime(&time));
    // hour number
    int hour = atoi(s_hour);
    bool evenHours = hour % 2 == 0;

    // time lines every 6h (or 12h)
    bool renderline = (evenHours && hour % 6 == 0) || (!evenHours && hour % 6 == 1);
    bool rendermarker = renderline;
    if (settings.forecastTimeInterval == '3') {
      rendermarker = (evenHours && hour % 12 == 0) || (!evenHours && hour % 12 == 1);
    }

    // x-position, move x back 1 hour if odd hours
    int16_t x = (i - (hour % 2)) * timeInterval / 1000 + chartPaddingX;

    // draw vertical line
    if (settings.tempGrid && renderline) {
      graphics_draw_line(ctx, GPoint(x, 0), GPoint(x, F_HEIGHT - chartPaddingY - 1));
    }

    // hour number below X
    if (rendermarker) {
      if (settings.forecastTimeInterval == '3') {
        // 12h AM/PM marker
        if (hour == 0 || hour == 1) {
          strcpy(s_hour, "12AM");
        } else {
          strcpy(s_hour, "12PM");
        }
        rect_bounds = GRect(x - 12, F_HEIGHT - 11, 24, 10);
      } else {
        snprintf(s_hour, sizeof(s_hour), "%d", hour - (hour % 2));
        rect_bounds = GRect(x - 6, F_HEIGHT - 11, 12, 10);
      }
      graphics_draw_text(ctx, s_hour, font, rect_bounds, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    }
  }

  // Condition colors
  for (int i = 0; i < forecastSize; i++) {
    int16_t x = i * timeInterval / 1000 + chartPaddingX + 1;
    GColor c = GColorBlack;
    bool render = false;

    switch (forecast[i].condition) {
      case GenericWeatherConditionClearSky:
      case GenericWeatherConditionScatteredClouds:
      case GenericWeatherConditionFewClouds:
#if defined(PBL_COLOR)
        c = GColorYellow;
#endif
        break;
      case GenericWeatherConditionBrokenClouds:
      case GenericWeatherConditionMist:
      case GenericWeatherConditionWind:
#if defined(PBL_COLOR)
        c = GColorDarkGray;
#endif
        break;
      case GenericWeatherConditionShowerRain:
      case GenericWeatherConditionRain:
      case GenericWeatherConditionSnow:
      case GenericWeatherConditionThunderstorm:
#if defined(PBL_COLOR)
        c = GColorVividCerulean;
#else
        c = GColorWhite;
        render = true;
#endif
        break;
      default:
        c = GColorBlack;
    }
#if defined(PBL_COLOR)
    graphics_context_set_fill_color(ctx, c);
    GRect conditionRect = GRect(x, F_HEIGHT - chartPaddingY - 5, timeInterval / 1000 + 1, 5);
    graphics_fill_rect(ctx, conditionRect, 0, GCornerNone);
#else
    if (render) {
      graphics_context_set_fill_color(ctx, c);
      GRect conditionRect = GRect(x, F_HEIGHT - chartPaddingY - 5, timeInterval / 1000 + 1, 5);
      graphics_fill_rect(ctx, conditionRect, 0, GCornerNone);
    }
#endif
  }

  // Temp line graph
  GPoint start = GPoint(-1, -1);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  for (int i = 0; i < forecastSize; i++) {
    int16_t x = i * timeInterval / 1000 + 1;
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
  GFont fontY = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  static char s_temp_text[6];
  rect_bounds = GRect(-2, -4, 17, 10);
  snprintf(s_temp_text, sizeof(s_temp_text), "%d", params.maxValue);
  graphics_draw_text(ctx, s_temp_text, fontY, rect_bounds, GTextOverflowModeFill, GTextAlignmentRight, NULL);

  rect_bounds = GRect(-2, F_HEIGHT - 13 - chartPaddingY, 17, 10);
  snprintf(s_temp_text, sizeof(s_temp_text), "%d", params.minValue);
  graphics_draw_text(ctx, s_temp_text, fontY, rect_bounds, GTextOverflowModeFill, GTextAlignmentRight, NULL);
}

void handle_weather(bool refresh) {
  // old weather data
  GenericWeatherPeekData peek = generic_weather_peek();
  bool peek_available = peek.info && peek.info->timestamp;
  if (peek_available && !userIsSleeping()) {
    // check that at least 1 hour has passed
    time_t now = time(NULL);
    if (now - peek.info->timestamp >= 3600) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Weather updating, last time was %d sec ago", (int)(now - peek.info->timestamp));
      refresh = true;
    }
  }
  // no peek data, or refresh requested
  if (!peek_available || refresh) {
    // request new data from remote
    app_timer_register(3000, js_ready_handler, NULL);
  } else {
    // use old data
    weather_callback(peek.info, peek.forecast, peek.forecastSize, GenericWeatherStatusAvailable);
  }
}

void hide_weather(bool hide) {
  layer_set_hidden((Layer *)s_weather_text_layer, hide);
  layer_set_hidden((Layer *)s_weather_loc_layer, hide);
  layer_set_hidden((Layer *)s_weather_unit_layer, hide);
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

  layer_set_update_proc(s_forecast_container, forecast_update_proc);

  handle_weather(false);
}

void weather_unload() {
  text_layer_destroy(s_weather_icon_layer);
  text_layer_destroy(s_weather_text_layer);
  text_layer_destroy(s_weather_unit_layer);
  text_layer_destroy(s_weather_loc_layer);
  fonts_unload_custom_font(weatherFont);
  fonts_unload_custom_font(font24);
}

void weather_init() {
  generic_weather_init();
  generic_weather_set_forecast(true);
  // generic_weather_set_location( (GenericWeatherCoordinates) { 6046999, 2508666 } );
  generic_weather_load(WEATHER_KEY);
  if (settings.weatherProvider == 'w' || settings.weatherProvider == 'o') {
    generic_weather_load_forecast(WEATHER_KEY_FORECAST);
  }
}

void weather_deinit() {
  generic_weather_deinit();
}