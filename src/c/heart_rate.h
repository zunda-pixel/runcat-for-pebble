#pragma once

#include <pebble.h>

typedef struct {
  int bpm;
  bool available;
  bool subscribed;
} HeartRate;

typedef void (*HeartRateChangedHandler)(int bpm, bool available, void *context);

void heart_rate_init(HeartRate *heart_rate, HeartRateChangedHandler handler,
                     void *context);
void heart_rate_deinit(HeartRate *heart_rate);
