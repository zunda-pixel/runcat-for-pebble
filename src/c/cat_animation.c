#include "cat_animation.h"

static CatAnimation *s_animation;

static void cat_layer_update(Layer *layer, GContext *ctx) {
  if (!s_animation || !s_animation->frames[s_animation->current_frame]) {
    return;
  }

  // The source PNGs have a black transparent background after conversion on
  // monochrome Pebbles. Assigning them preserves the white cat silhouette.
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  graphics_draw_bitmap_in_rect(ctx,
                               s_animation->frames[s_animation->current_frame],
                               layer_get_bounds(layer));
}

static void cat_animation_tick(void *context) {
  CatAnimation *animation = context;
  animation->current_frame = (animation->current_frame + 1) % animation->frame_count;
  layer_mark_dirty(animation->layer);
  animation->timer = app_timer_register(animation->frame_interval_ms,
                                        cat_animation_tick, animation);
}

bool cat_animation_init(CatAnimation *animation, Layer *root, GRect frame_rect,
                        const uint32_t *resource_ids, uint8_t frame_count,
                        uint32_t initial_interval_ms) {
  if (!animation || !root || !resource_ids || frame_count == 0 || frame_count > 5) {
    return false;
  }

  *animation = (CatAnimation){
    .frame_rect = frame_rect,
    .frame_count = frame_count,
    .frame_interval_ms = initial_interval_ms,
  };

  for (uint8_t i = 0; i < frame_count; i++) {
    animation->frames[i] = gbitmap_create_with_resource(resource_ids[i]);
    if (!animation->frames[i]) {
      cat_animation_deinit(animation);
      return false;
    }
  }

  animation->layer = layer_create(frame_rect);
  if (!animation->layer) {
    cat_animation_deinit(animation);
    return false;
  }

  s_animation = animation;
  layer_set_update_proc(animation->layer, cat_layer_update);
  layer_add_child(root, animation->layer);
  animation->timer = app_timer_register(animation->frame_interval_ms,
                                        cat_animation_tick, animation);
  return animation->timer != NULL;
}

void cat_animation_deinit(CatAnimation *animation) {
  if (!animation) {
    return;
  }

  if (animation->timer) {
    app_timer_cancel(animation->timer);
    animation->timer = NULL;
  }

  if (animation->layer) {
    layer_destroy(animation->layer);
    animation->layer = NULL;
  }

  for (uint8_t i = 0; i < animation->frame_count; i++) {
    if (animation->frames[i]) {
      gbitmap_destroy(animation->frames[i]);
      animation->frames[i] = NULL;
    }
  }

  if (s_animation == animation) {
    s_animation = NULL;
  }
}

void cat_animation_set_interval(CatAnimation *animation, uint32_t interval_ms) {
  if (animation) {
    animation->frame_interval_ms = interval_ms;
  }
}
