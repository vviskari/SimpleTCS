#include "utils.h"
#include <pebble.h>

bool userIsSleeping() {
  bool isSleeping = false;
#if defined(PBL_HEALTH)
  // Get an activities mask
  HealthActivityMask activities = health_service_peek_current_activities();

  // Determine which bits are set, and hence which activity is active
  if (activities & HealthActivitySleep) {
    isSleeping = true;
  }
  if (activities & HealthActivityRestfulSleep) {
    isSleeping = true;
  }
#endif
  return isSleeping;
}

int get_time_hour(time_t time) {
    char s_hour[5];
    strftime(s_hour, 3, "%H", localtime(&time));
    int hour = atoi(s_hour);
    return hour;
}