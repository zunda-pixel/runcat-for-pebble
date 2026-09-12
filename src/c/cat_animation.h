#pragma once

#include <pebble.h>

typedef struct {
  Layer *layer;
  GBitmap *frames[5];
  uint8_t frame_count;
  uint8_t current_frame;
  uint32_t frame_interval_ms;
  AppTimer *timer;
  GRect frame_rect;
} CatAnimation;

bool cat_animation_init(CatAnimation *animation, Layer *root, GRect frame_rect,
                        const uint32_t *resource_ids, uint8_t frame_count,
                        uint32_t initial_interval_ms);
void cat_animation_deinit(CatAnimation *animation);
void cat_animation_set_interval(CatAnimation *animation, uint32_t interval_ms);
