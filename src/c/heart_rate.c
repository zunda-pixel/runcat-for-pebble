#include "heart_rate.h"

#if defined(PBL_HEALTH)
static HeartRate *s_heart_rate;
static HeartRateChangedHandler s_handler;
static void *s_context;

static void notify_changed(void) {
  if (s_handler) {
    s_handler(s_heart_rate->bpm, s_heart_rate->available, s_context);
  }
}

static void read_current_value(void) {
  HealthServiceAccessibilityMask access = health_service_metric_accessible(
      HealthMetricHeartRateBPM, time(NULL), time(NULL));

  if (!(access & HealthServiceAccessibilityMaskAvailable)) {
    s_heart_rate->bpm = 0;
    s_heart_rate->available = false;
    notify_changed();
    return;
  }

  HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
  s_heart_rate->bpm = value > 0 ? (int)value : 0;
  s_heart_rate->available = value > 0;
  notify_changed();
}

static void health_handler(HealthEventType event, void *context) {
  if (event == HealthEventHeartRateUpdate || event == HealthEventSignificantUpdate) {
    read_current_value();
  }
}
#endif

void heart_rate_init(HeartRate *heart_rate, HeartRateChangedHandler handler,
                     void *context) {
  *heart_rate = (HeartRate){0};

#if defined(PBL_HEALTH)
  s_heart_rate = heart_rate;
  s_handler = handler;
  s_context = context;

  read_current_value();
  if (health_service_events_subscribe(health_handler, NULL)) {
    heart_rate->subscribed = true;
  }

  // One-minute updates make the speed change visible while avoiding high-rate sampling.
  health_service_set_heart_rate_sample_period(60);
#else
  if (handler) {
    handler(0, false, context);
  }
#endif
}

void heart_rate_deinit(HeartRate *heart_rate) {
#if defined(PBL_HEALTH)
  health_service_set_heart_rate_sample_period(0);
  if (heart_rate && heart_rate->subscribed) {
    health_service_events_unsubscribe();
    heart_rate->subscribed = false;
  }
  s_heart_rate = NULL;
  s_handler = NULL;
  s_context = NULL;
#else
  (void)heart_rate;
#endif
}
