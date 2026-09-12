#include <pebble.h>

#include "cat_animation.h"
#include "heart_rate.h"
#include "watchface.h"

static Watchface s_watchface;

int main(void) {
  watchface_init(&s_watchface);
  app_event_loop();
  watchface_deinit(&s_watchface);
}
