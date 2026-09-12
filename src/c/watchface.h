#pragma once

#include <pebble.h>

#include "cat_animation.h"
#include "heart_rate.h"

typedef struct {
  Window *window;
  TextLayer *time_layer;
  TextLayer *date_layer;
  TextLayer *heart_rate_layer;
  Layer *heart_icon_layer;
  char heart_rate_text[16];
  CatAnimation cat;
  HeartRate heart_rate;
} Watchface;

void watchface_init(Watchface *watchface);
void watchface_deinit(Watchface *watchface);
