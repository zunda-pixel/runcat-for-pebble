#include "watchface.h"

enum {
  DEFAULT_CAT_INTERVAL_MS = 350,
  CAT_MIN_INTERVAL_MS = 50,
  CAT_MAX_INTERVAL_MS = 450,
};

static const uint32_t s_cat_resources[] = {
  RESOURCE_ID_CAT_0,
  RESOURCE_ID_CAT_1,
  RESOURCE_ID_CAT_2,
  RESOURCE_ID_CAT_3,
  RESOURCE_ID_CAT_4,
};

static Watchface *s_watchface;

static uint32_t animation_interval_for_bpm(int bpm) {
  if (bpm <= 0) return DEFAULT_CAT_INTERVAL_MS;
  if (bpm < 60) return CAT_MAX_INTERVAL_MS;
  if (bpm < 80) return 350;
  if (bpm < 100) return 250;
  if (bpm < 120) return 140;
  if (bpm < 150) return 80;
  return CAT_MIN_INTERVAL_MS;
}

static void draw_heart_icon(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(bounds.size.w / 2 - 4, 7), 4);
  graphics_fill_circle(ctx, GPoint(bounds.size.w / 2 + 4, 7), 4);
  graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 8, 7, 16, 7), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 5, 11, 10, 5), 0, GCornerNone);
}

static void update_heart_rate(Watchface *watchface, int bpm, bool available) {
  watchface->heart_rate.bpm = bpm;
  watchface->heart_rate.available = available;
  cat_animation_set_interval(&watchface->cat, animation_interval_for_bpm(bpm));
  if (available) {
    snprintf(watchface->heart_rate_text, sizeof(watchface->heart_rate_text), "%d BPM", bpm);
  } else {
    snprintf(watchface->heart_rate_text, sizeof(watchface->heart_rate_text), "-- BPM");
  }
  text_layer_set_text(watchface->heart_rate_layer, watchface->heart_rate_text);
}

static void health_changed(int bpm, bool available, void *context) {
  update_heart_rate(context, bpm, available);
}

static void update_time(Watchface *watchface) {
  time_t now = time(NULL);
  struct tm *local = localtime(&now);
  static char time_buffer[8];
  static char date_buffer[16];
  const char *format = clock_is_24h_style() ? "%H:%M" : "%I:%M";

  strftime(time_buffer, sizeof(time_buffer), format, local);
  strftime(date_buffer, sizeof(date_buffer), "%a %d %b", local);
  text_layer_set_text(watchface->time_layer, time_buffer);
  text_layer_set_text(watchface->date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed & MINUTE_UNIT) {
    update_time(s_watchface);
  }
}

static void window_load(Window *window) {
  Watchface *watchface = window_get_user_data(window);
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  const int width = bounds.size.w;
  const int height = bounds.size.h;
  // graphics_draw_bitmap_in_rect clips rather than scales. Keep the official
  // 126x77 frame size so every supported rectangular display shows it intact.
  const int cat_width = 126;
  const int cat_height = 77;
  const int cat_y = height - cat_height - 34;
  const int cat_x = (width - cat_width) / 2 - 6;

  // Give the large clock glyphs enough vertical room so their lower edges
  // remain visible on every rectangular display.
  watchface->time_layer = text_layer_create(GRect(0, 8, width, 70));
  watchface->date_layer = text_layer_create(GRect(0, 78, width, 28));
  // The heart sits just left of the centered value; shift the complete BPM
  // group slightly right so the visual group is centered as a whole.
  watchface->heart_rate_layer = text_layer_create(GRect(14, height - 31, width, 24));
  watchface->heart_icon_layer = layer_create(GRect(width / 2 - 33, height - 28, 18, 18));

  text_layer_set_background_color(watchface->time_layer, GColorClear);
  text_layer_set_text_color(watchface->time_layer, GColorWhite);
  text_layer_set_font(watchface->time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(watchface->time_layer, GTextAlignmentCenter);

  text_layer_set_background_color(watchface->date_layer, GColorClear);
  text_layer_set_text_color(watchface->date_layer, GColorLightGray);
  text_layer_set_font(watchface->date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(watchface->date_layer, GTextAlignmentCenter);

  text_layer_set_background_color(watchface->heart_rate_layer, GColorClear);
  text_layer_set_text_color(watchface->heart_rate_layer, GColorWhite);
  text_layer_set_font(watchface->heart_rate_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(watchface->heart_rate_layer, GTextAlignmentCenter);
  layer_set_update_proc(watchface->heart_icon_layer, draw_heart_icon);

  layer_add_child(root, text_layer_get_layer(watchface->time_layer));
  layer_add_child(root, text_layer_get_layer(watchface->date_layer));
  layer_add_child(root, text_layer_get_layer(watchface->heart_rate_layer));
  layer_add_child(root, watchface->heart_icon_layer);

  cat_animation_init(&watchface->cat, root,
                     GRect(cat_x, cat_y, cat_width, cat_height),
                     s_cat_resources, ARRAY_LENGTH(s_cat_resources),
                     DEFAULT_CAT_INTERVAL_MS);
  update_time(watchface);
  heart_rate_init(&watchface->heart_rate, health_changed, watchface);
}

static void window_unload(Window *window) {
  Watchface *watchface = window_get_user_data(window);
  heart_rate_deinit(&watchface->heart_rate);
  cat_animation_deinit(&watchface->cat);
  layer_destroy(watchface->heart_icon_layer);
  text_layer_destroy(watchface->heart_rate_layer);
  text_layer_destroy(watchface->date_layer);
  text_layer_destroy(watchface->time_layer);
  watchface->heart_icon_layer = NULL;
  watchface->heart_rate_layer = NULL;
  watchface->date_layer = NULL;
  watchface->time_layer = NULL;
}

void watchface_init(Watchface *watchface) {
  *watchface = (Watchface){0};
  s_watchface = watchface;
  watchface->window = window_create();
  window_set_background_color(watchface->window, GColorBlack);
  window_set_user_data(watchface->window, watchface);
  window_set_window_handlers(watchface->window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(watchface->window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

void watchface_deinit(Watchface *watchface) {
  tick_timer_service_unsubscribe();
  s_watchface = NULL;
  if (watchface->window) {
    window_destroy(watchface->window);
    watchface->window = NULL;
  }
}
