//
// Simple TCS watchface for Pebble (Time series)
//
// Based on "Simple Time and Calendar" by Michael S.
// 
//

#include <pebble.h>

#include <pebble-generic-weather/pebble-generic-weather.h>
#include <pebble-events/pebble-events.h>

static Window *s_main_window;
static TextLayer *s_connection_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_box;
static TextLayer *s_date_layer;
static TextLayer *s_time_layer;
static TextLayer *s_seconds_layer;
static TextLayer *s_cal_array_layer[3][7];
static TextLayer *s_bat_cal_bg_layer[3];
static TextLayer *s_bat_cal_bat_layer[3];
static TextLayer *s_weather_text_layer;
static TextLayer *s_weather_icon_layer;
static TextLayer *s_weather_unit_layer;
static TextLayer *s_weather_loc_layer;

static GBitmap *s_battery_bitmap;
static GBitmap *s_bluetooth_bitmap;
static BitmapLayer *s_battery_bitmap_layer;
static BitmapLayer *s_bluetooth_bitmap_layer;

#if defined(PBL_HEALTH)
static BitmapLayer *s_shoe_bitmap_layer;
static GBitmap *s_shoe_bitmap;
static TextLayer *s_steps_layer;
#endif

const int weekSeconds = 60*60*24*7;
static bool lastconnected = true;
static int secondticks = 0;
static int shakes = 0;
static bool show_seconds = false;
static int shaketicks = 0;
static int MAX_SECONDS = 60;
static char weekdaynumber[] = "0";

char weekdayname[6][7][15] = {
  {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"},
  {"Montag","Dienstag","Mittwoch","Donnerstag","Freitag","Samstag","Sonntag"},
  {"lunes","martes","miércoles","jueves","viernes","sábado","domingo"},
  {"Lundi","Mardi","Mercredi","Jeudi","Vendredi","Samedi","Dimanche"},
  {"Lunedì","Martedì","Mercoledì","Giovedì","Venerdì","Sabato","Domenica"},
  {"Segunda","Terça","Quarta","Quinta","Sexta","Sábado","Domingo"}
};

typedef struct {
  time_t timestamp;
  int charge;
  int last_estimated;
  int slope;
} BatteryHistory;

typedef struct {
  char weekStartDay;
  char weatherTemp;
  char weatherProvider;
  char weatherApiKey[20];
} Settings;

static Settings settings;

const uint32_t BATT_HISTORY_KEY = 1;
const uint32_t BATT_CHARGING_KEY = 2;
const uint32_t WEATHER_KEY = 3;
const uint32_t SETTINGS_KEY = 4;

const char WEATHER_CLEAR = 'B';
const char WEATHER_CLEAR_NIGHT = 'C';
const char WEATHER_PART_CLOUD = 'H';
const char WEATHER_PART_CLOUD_NIGHT = 'I';
const char WEATHER_FOG = 'M';
const char WEATHER_CLOUD = 'N';
const char WEATHER_STORM = 'P';
const char WEATHER_LIGHT_RAIN = 'Q';
const char WEATHER_RAIN = 'R';
const char WEATHER_SNOW = 'W';
const char WEATHER_WIND = 'F';
const char WEATHER_UNKNOWN = ')';

static void render_weather(GenericWeatherInfo *info) {
  if (!info) {
    return;
  }
  //APP_LOG(APP_LOG_LEVEL_INFO, "Weather, %s, %s, %d", info->name, info->description, info->temp_c);
  static char s_temp_text[5];
  if (settings.weatherTemp == 'C') {
    snprintf(s_temp_text, sizeof(s_temp_text), "%d˚",  info->temp_c);      
  } else {
    snprintf(s_temp_text, sizeof(s_temp_text), "%d˚",  info->temp_f);
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

static void weather_callback(GenericWeatherInfo *info, GenericWeatherStatus status) {
  if (status == GenericWeatherStatusAvailable) {
    generic_weather_save(WEATHER_KEY);
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
  generic_weather_fetch(weather_callback);
}

static bool userIsSleeping() {
  bool isSleeping = false;
  #if defined(PBL_HEALTH)
  // Get an activities mask
  HealthActivityMask activities = health_service_peek_current_activities();
  
  // Determine which bits are set, and hence which activity is active
  if(activities & HealthActivitySleep) {
    isSleeping = true;
  }
  if (activities & HealthActivityRestfulSleep) {
    isSleeping = true;
  }
  #endif
  return isSleeping;
}

static void handle_weather(bool refresh) {
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

static void estimate_battery(BatteryChargeState charge_state) {
  time_t now = time(NULL);
  
  // Store current charging state
  bool lastCharging = false;
  if (persist_exists(BATT_CHARGING_KEY)) {
    lastCharging = persist_read_bool(BATT_CHARGING_KEY);
  }
  persist_write_bool(BATT_CHARGING_KEY, charge_state.is_plugged);

  // skip everything, if charging
  if (charge_state.is_plugged) {
    // APP_LOG(APP_LOG_LEVEL_INFO, "Charging, skip");
    return;
  }
  
  // default history
  BatteryHistory history = (BatteryHistory) { 
    .timestamp = 0, // marks null history if time is 0
    .charge = charge_state.charge_percent, // charge when last disconnected
    .last_estimated = 0, // charge when last estimated battery life
    .slope = -694 // Initial estimate for full battery is 6d
  };

  if (persist_exists(BATT_HISTORY_KEY)) {
    persist_read_data(BATT_HISTORY_KEY, &history, sizeof(BatteryHistory));
    //APP_LOG(APP_LOG_LEVEL_INFO, "Loaded history, charge=%d, slope=%d", history.charge, history.slope);
  }

  if (lastCharging && !charge_state.is_plugged) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "Charger disconnected. Store state to history");
    history.timestamp = now;
    history.charge = charge_state.charge_percent;
  }
  
  if (history.timestamp == 0) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "No old history. return");
    return;
  }

  int historyTimeHours = history.timestamp/60/60;
  int historyChargePercent = history.charge*1000;

  if (history.last_estimated != charge_state.charge_percent && charge_state.charge_percent < 90) {
    // new estimation
    // skip battery levels 100 and 90, they are not reliable
    int currentTimeHours = (int)now/60/60;
    int currentChargePercent = charge_state.charge_percent*1000;  
    
    // calculate slope for current charge against history charge (weighted)
    history.slope = (2*history.slope + (currentChargePercent-historyChargePercent)/(currentTimeHours-historyTimeHours)) / 3;
    history.last_estimated = charge_state.charge_percent;
  }
    
  // estimated time when battery is 0
  int estimatedBatteryLife = (-historyChargePercent + history.slope * historyTimeHours) / history.slope*60*60;
  //APP_LOG(APP_LOG_LEVEL_INFO, "Estimated battery life until %d", estimatedBatteryLife);

  persist_write_data(BATT_HISTORY_KEY, &history, sizeof(BatteryHistory));

  ///////////////
  // start render

  // current week day 0-6
  char weekdaynumber[2];
  strftime(weekdaynumber, 2, "%u", localtime(&now));
  struct tm *stm = localtime(&now);
  int offset = (settings.weekStartDay == 's') ? 0 : 1;
  int weekStart = now - (atoi(weekdaynumber)-offset)*60*60*24 - stm->tm_hour*60*60 - stm->tm_min*60 - stm->tm_sec;
  
  Layer *window_layer = window_get_root_layer(s_main_window);
  text_layer_destroy(s_bat_cal_bat_layer[0]);
  text_layer_destroy(s_bat_cal_bat_layer[1]);
  text_layer_destroy(s_bat_cal_bat_layer[2]);

  // last week
  if (history.timestamp < weekStart) {
    // history starts last week
    int start = 140 - (weekStart-history.timestamp)*140/weekSeconds;
    if (start < 0) {
      start = 0;
    }
    int width = 140 - start;
    //APP_LOG(APP_LOG_LEVEL_INFO, "Last week %d, %d", start, width);
    s_bat_cal_bat_layer[0] = text_layer_create(GRect(2+start, 134, width, 2));
    text_layer_set_background_color(s_bat_cal_bat_layer[0], GColorSpringBud);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bat_layer[0]));
  } else {
    // no history last week
    s_bat_cal_bat_layer[0] = text_layer_create(GRect(2, 134, 140, 2));
    text_layer_set_background_color(s_bat_cal_bat_layer[0], GColorBlack);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bat_layer[0]));
  }

  // this week
  if (true) {
    int start = 0;
    if (history.timestamp > weekStart) {
      // history starts this week
      start = (history.timestamp-weekStart)*140/weekSeconds;
    }
    int width = 140 - start;
    if (estimatedBatteryLife-weekStart < weekSeconds) {
      // estimation ends this week
      width = (estimatedBatteryLife-weekStart)*140/weekSeconds - start;
    }
    //APP_LOG(APP_LOG_LEVEL_INFO, "This week %d, %d", start, width);
    s_bat_cal_bat_layer[1] = text_layer_create(GRect(2+start, 134+15, width, 2));
    text_layer_set_background_color(s_bat_cal_bat_layer[1], GColorSpringBud);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bat_layer[1]));
  }
  
  // next week
  if (estimatedBatteryLife > weekStart+weekSeconds) {
    // estimation ends next week
    int width = (estimatedBatteryLife-weekStart-weekSeconds)*140/weekSeconds;
    if (width > 140) {
      width = 140;
    }
    //APP_LOG(APP_LOG_LEVEL_INFO, "Next week %d", width);

    s_bat_cal_bat_layer[2] = text_layer_create(GRect(2, 134+30, width, 2));
    text_layer_set_background_color(s_bat_cal_bat_layer[2], GColorSpringBud);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bat_layer[2]));
  } else {
    // black next week
    s_bat_cal_bat_layer[2] = text_layer_create(GRect(2, 134+30, 140, 2));
    text_layer_set_background_color(s_bat_cal_bat_layer[2], GColorBlack);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bat_layer[2]));
  }
}

static bool is_day_idx_weekend(int idx) {
  if (settings.weekStartDay == 's') {    
    if (idx%7 == 0 || idx%7 == 6) {
      return true;
    }
  } else {
    if (idx%7>=5) {
      return true;
    }    
  }
  return false;
}

static void drawcal() {
  static char monthdaynumber[3];
  char weekdaynumber[2];
  time_t now = time(NULL);
  // Day of the month (01-31)
  strftime(monthdaynumber, 3, "%d", localtime(&now));
  // The weekday as a number, 1-based from Monday (from ‘1’ to ‘7’). [tm_wday]
  strftime(weekdaynumber, 2, "%u", localtime(&now));
  
  static char week[3][7][3];

  int day_id = 0;
  int offset = (settings.weekStartDay=='s') ? 0 : 1;

  // start from beginning of last week (depending if week starts on mon or sun)
  for (int x = offset-atoi(weekdaynumber)-7; x < 14+offset-atoi(weekdaynumber); x++){
    // timestamp adjusted by x days
    time_t tt = time(NULL)+x*24*3600;
    struct tm *stm = localtime(&tt);
    if (day_id >= 14) {
      if (is_day_idx_weekend(day_id)) {
        text_layer_set_font(s_cal_array_layer[2][day_id-14], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
      } else {
        text_layer_set_font(s_cal_array_layer[2][day_id-14], fonts_get_system_font(FONT_KEY_GOTHIC_18));
      }
      strftime(week[2][day_id-14], 3, "%d", stm);
      text_layer_set_text(s_cal_array_layer[2][day_id-14], week[2][day_id-14]);
    } else if (day_id >= 7) {
      if (is_day_idx_weekend(day_id)) {
        text_layer_set_font(s_cal_array_layer[1][day_id-7], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
      } else {
        text_layer_set_font(s_cal_array_layer[1][day_id-7], fonts_get_system_font(FONT_KEY_GOTHIC_18));
      }
      strftime(week[1][day_id-7], 3, "%d", stm);
      text_layer_set_text(s_cal_array_layer[1][day_id-7], week[1][day_id-7]);

      if (strcmp(monthdaynumber, week[1][day_id-7]) == 0) {
        #if defined(PBL_COLOR)
        text_layer_set_font(s_cal_array_layer[1][day_id-7], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
        text_layer_set_background_color(s_cal_array_layer[1][day_id-7], GColorDarkCandyAppleRed);
        #else
        text_layer_set_font(s_cal_array_layer[1][day_id-7], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
        text_layer_set_background_color(s_cal_array_layer[1][day_id-7], GColorWhite);
        text_layer_set_text_color(s_cal_array_layer[1][day_id-7], GColorBlack);        
        #endif
      } else {
        text_layer_set_background_color(s_cal_array_layer[1][day_id-7], GColorClear);
        text_layer_set_text_color(s_cal_array_layer[1][day_id-7], GColorWhite);
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

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";
  
  Layer *window_layer = window_get_root_layer(s_main_window);
  text_layer_destroy(s_box);
  int width = 22*charge_state.charge_percent/100;
  s_box = text_layer_create(GRect(106-width, 4, width, 9));
  layer_add_child(window_layer, text_layer_get_layer(s_box));

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "%d%%+", charge_state.charge_percent);
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
    text_layer_set_text_color(s_battery_layer, GColorRed);
    text_layer_set_background_color(s_box, GColorRed);
    if (charge_state.charge_percent>20) {
      text_layer_set_text_color(s_battery_layer, GColorYellow);
      text_layer_set_background_color(s_box, GColorYellow);
    }
    if (charge_state.charge_percent>40) {
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
      static uint32_t const segments[] = { 300, 100, 100, 100, 100 };
      VibePattern pat = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
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
      static uint32_t const segments[] = { 300, 300, 300, 300, 300 };
      VibePattern pat = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
      };
      vibes_enqueue_custom_pattern(pat);
    }
  }
  s_bluetooth_bitmap_layer = bitmap_layer_create(GRect(3, 2, 8, 13));
  bitmap_layer_set_bitmap(s_bluetooth_bitmap_layer, s_bluetooth_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bluetooth_bitmap_layer));
}

static void handle_steps() {
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

static void handle_time(struct tm* tick_time, TimeUnits units_changed) {
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

static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
  handle_time(tick_time, units_changed);
  
  if (tick_time->tm_min == 0) {
    // Draw calendar every hour on the hour
    drawcal();
    if (tick_time->tm_hour == 0) {
      // refresh battery stats at midnight
      handle_battery(battery_state_service_peek());
    }
  }
  // Steps every 5 mins
  if(tick_time->tm_min % 5 == 0) {
    handle_steps();
  }
  // Update weather twice an hour
  if(tick_time->tm_min % 30 == 0) {
    handle_weather(true);
  }
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  secondticks++;
  shaketicks++;
  handle_time(tick_time, units_changed);

  if (shaketicks % 3 == 0 && shakes == 1) {
    shaketicks = shakes = 0;
  } else if (shakes > 1) {
    shakes = 0;
    show_seconds = !show_seconds;
    secondticks= show_seconds ? 1 : MAX_SECONDS+1;    
  }
  
  if (show_seconds && secondticks <= MAX_SECONDS) {
    static char s_seconds_text[] = "00";
    strftime(s_seconds_text, sizeof(s_seconds_text), "%S", tick_time);
    text_layer_set_text(s_seconds_layer, s_seconds_text);
  
    layer_set_hidden((Layer*) s_weather_text_layer, true);
    layer_set_hidden((Layer*) s_weather_loc_layer, true);
    layer_set_hidden((Layer*) s_weather_icon_layer, true);
    layer_set_hidden((Layer*) s_seconds_layer, false);
  } else {
    text_layer_set_text(s_seconds_layer, "");
    layer_set_hidden((Layer*) s_weather_text_layer, false);
    layer_set_hidden((Layer*) s_weather_loc_layer, false);
    layer_set_hidden((Layer*) s_weather_icon_layer, false);
    layer_set_hidden((Layer*) s_seconds_layer, true);    
  }

  if (secondticks > MAX_SECONDS) {
    secondticks=0;
    show_seconds = false;
    tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  }  
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  shakes = (shakes+1) % 3;
  shaketicks = 0;
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

// Save the settings to persistent storage
static void save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void load_settings() {
  if (persist_exists(SETTINGS_KEY)) {
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  }
}

static void conf_inbox_received_handler(DictionaryIterator *iter, void *context) {
  bool updateWeather = false;
  
  Tuple *conf = dict_find(iter, MESSAGE_KEY_wsd);
  if(conf) {
    char old = settings.weekStartDay;
    settings.weekStartDay = conf->value->cstring[0];
    if (old != settings.weekStartDay) {
      drawcal();
      handle_battery(battery_state_service_peek());
    }
  }

  conf = dict_find(iter, MESSAGE_KEY_wt);
  if (conf) {
    settings.weatherTemp = conf->value->cstring[0];
  }

  conf = dict_find(iter, MESSAGE_KEY_wp);
  if (conf) {
    char old = settings.weatherProvider;
    settings.weatherProvider = conf->value->cstring[0];
    if (old != settings.weatherProvider) {
      updateWeather = true;
    }
  }

  conf = dict_find(iter, MESSAGE_KEY_wak);
  if (conf) {
    char old[20];
    strncpy(old, settings.weatherApiKey, 20);
    strncpy(settings.weatherApiKey, conf->value->cstring, 20);
    if (strcmp(old, settings.weatherApiKey)!=0) {
      updateWeather = true;
    }
  }

  save_settings();
  handle_weather(updateWeather);  
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_connection_layer = text_layer_create(GRect(16, -5, bounds.size.w, 18));
  text_layer_set_text_color(s_connection_layer, GColorWhite);
  text_layer_set_background_color(s_connection_layer, GColorClear);
  text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  handle_bluetooth(connection_service_peek_pebble_app_connection());

  s_battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BAT2);
  s_battery_bitmap_layer = bitmap_layer_create(GRect(80, 2, 28, 13));
  bitmap_layer_set_bitmap(s_battery_bitmap_layer, s_battery_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_battery_bitmap_layer));

  s_battery_layer = text_layer_create(GRect(109, -4, 36, 18));
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

  s_date_layer = text_layer_create(GRect(0, 16, 144, 34));
  text_layer_set_text_color(s_date_layer, GColorYellow);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  s_time_layer = text_layer_create(GRect(0, 28, 144, 55));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_52)));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  s_seconds_layer = text_layer_create(GRect(100, 80, 50, 40));
  #if defined(PBL_BW)
  text_layer_set_text_color(s_seconds_layer, GColorWhite);
  #else
  text_layer_set_text_color(s_seconds_layer, GColorOrange);
  #endif
  text_layer_set_background_color(s_seconds_layer, GColorClear);

  text_layer_set_font(s_seconds_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_32)));
  layer_set_hidden((Layer *) s_seconds_layer, true);
  
  for (int week=2;week>=0;week--){
    for (int day=0; day<7; day++) {
      s_cal_array_layer[week][day] = text_layer_create(GRect(2+day*20, 115+week*15, 20, 21));
      text_layer_set_text_color(s_cal_array_layer[week][day], GColorWhite);
      text_layer_set_background_color(s_cal_array_layer[week][day], GColorClear);
      text_layer_set_font(s_cal_array_layer[week][day], fonts_get_system_font(FONT_KEY_GOTHIC_18));
      text_layer_set_text_alignment(s_cal_array_layer[week][day], GTextAlignmentCenter);
      text_layer_set_text(s_cal_array_layer[week][day], "00");
      layer_add_child(window_layer, text_layer_get_layer(s_cal_array_layer[week][day]));
    }
    // Battery estimation bar
    s_bat_cal_bg_layer[week] = text_layer_create(GRect(2, 134+week*15, 140, 2));
    text_layer_set_background_color(s_bat_cal_bg_layer[week], GColorBlack);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bg_layer[week]));

    s_bat_cal_bat_layer[week] = text_layer_create(GRect(2, 134+week*15, 140, 2));
    text_layer_set_background_color(s_bat_cal_bat_layer[week], GColorBlack);
    layer_add_child(window_layer, text_layer_get_layer(s_bat_cal_bat_layer[week]));
  }
  drawcal();

  s_box = text_layer_create(GRect(0, 0, 1, 1));
  text_layer_set_background_color(s_box, GColorRed);
  layer_add_child(window_layer, text_layer_get_layer(s_box));

  #if defined(PBL_HEALTH)
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

  s_weather_icon_layer = text_layer_create(GRect(53, 85, 32, 35));
  text_layer_set_text_color(s_weather_icon_layer, GColorWhite);
  text_layer_set_background_color(s_weather_icon_layer, GColorClear);
  text_layer_set_font(s_weather_icon_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHER_30)));
  layer_add_child(window_layer, text_layer_get_layer(s_weather_icon_layer));

  s_weather_text_layer = text_layer_create(GRect(88, 83, 55, 30));
  text_layer_set_text_color(s_weather_text_layer, GColorWhite);
  text_layer_set_background_color(s_weather_text_layer, GColorClear);
  text_layer_set_font(s_weather_text_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_24)));
  text_layer_set_text_alignment(s_weather_text_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_text_layer));

  s_weather_unit_layer = text_layer_create(GRect(124, 92, 15, 15));
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
  
  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_minute_tick(current_time, MINUTE_UNIT);

  // SERVICE SUBSCRIPTIONS
  accel_tap_service_subscribe(tap_handler);
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  battery_state_service_subscribe(handle_battery);
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = handle_bluetooth
  });

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_seconds_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));

  handle_battery(battery_state_service_peek());
  handle_steps();
  handle_weather(false);
}

static void main_window_unload(Window *window) {
  // SERVICE UNSUBSCRIPTIONS
  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  
  text_layer_destroy(s_connection_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_box);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_seconds_layer);
  text_layer_destroy(s_weather_loc_layer);
  text_layer_destroy(s_weather_text_layer);
  text_layer_destroy(s_weather_unit_layer);
  text_layer_destroy(s_weather_icon_layer);
  
  gbitmap_destroy(s_battery_bitmap);
  gbitmap_destroy(s_bluetooth_bitmap);
  bitmap_layer_destroy(s_battery_bitmap_layer);
  bitmap_layer_destroy(s_bluetooth_bitmap_layer);

  #if defined(PBL_HEALTH)
  text_layer_destroy(s_steps_layer);
  gbitmap_destroy(s_shoe_bitmap);
  bitmap_layer_destroy(s_shoe_bitmap_layer);
  #endif

  // destroy calendar
  for (int week=2;week>=0;week--){
    for (int day=0; day<7; day++) {
      text_layer_destroy(s_cal_array_layer[week][day]);
    }
    text_layer_destroy(s_bat_cal_bg_layer[week]);
    text_layer_destroy(s_bat_cal_bat_layer[week]);
  }  
}

static void init() {
  s_main_window = window_create();

  settings.weekStartDay = 'm';
  settings.weatherTemp = 'C';
  settings.weatherProvider = 'y';

  load_settings();

  generic_weather_init();
  generic_weather_load(WEATHER_KEY);

  // Clay
  events_app_message_register_inbox_received(conf_inbox_received_handler, NULL);
  events_app_message_open();
  
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  generic_weather_deinit();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
