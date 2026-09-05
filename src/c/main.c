#include <pebble.h>

// Independent of AppMessage IDs and the original generator's storage namespace.
#define SETTINGS_KEY 2000
#define SETTINGS_VERSION 1

typedef struct {
  uint8_t version;
  uint8_t digit;
  uint8_t glow;
  uint8_t background;
} Settings;

static Window *s_window;
static Layer *s_face;
static GBitmap *s_digits[4];
static int8_t s_loaded[4] = {-1, -1, -1, -1};
static Settings s_settings;

static const uint32_t DIGIT_RESOURCES[] = {
  RESOURCE_ID_DIGIT_0, RESOURCE_ID_DIGIT_1, RESOURCE_ID_DIGIT_2,
  RESOURCE_ID_DIGIT_3, RESOURCE_ID_DIGIT_4, RESOURCE_ID_DIGIT_5,
  RESOURCE_ID_DIGIT_6, RESOURCE_ID_DIGIT_7, RESOURCE_ID_DIGIT_8,
  RESOURCE_ID_DIGIT_9
};

static void normalize_settings(void) {
#if defined(PBL_BW)
  // RGB values saved on another watch must still produce pure monochrome.
  s_settings.digit = gcolor_equal((GColor){.argb = s_settings.digit}, GColorBlack)
                      ? GColorBlack.argb : GColorWhite.argb;
  s_settings.background =
      gcolor_equal((GColor){.argb = s_settings.background}, GColorBlack)
        ? GColorBlack.argb : GColorWhite.argb;
  if (s_settings.digit == s_settings.background) {
    s_settings.digit = s_settings.background == GColorBlack.argb
                        ? GColorWhite.argb : GColorBlack.argb;
  }
#endif
}

static void load_settings(void) {
  s_settings = (Settings){
    .version = SETTINGS_VERSION,
    .digit = PBL_IF_COLOR_ELSE(GColorIcterine.argb, GColorWhite.argb),
    .glow = GColorYellow.argb,
    .background = PBL_IF_COLOR_ELSE(GColorOxfordBlue.argb, GColorBlack.argb)
  };
  Settings saved;
  if (persist_get_size(SETTINGS_KEY) == sizeof(saved) &&
      persist_read_data(SETTINGS_KEY, &saved, sizeof(saved)) == sizeof(saved) &&
      saved.version == SETTINGS_VERSION &&
      (saved.digit & 0xc0) == 0xc0 && (saved.glow & 0xc0) == 0xc0 &&
      (saved.background & 0xc0) == 0xc0) {
    s_settings = saved;
  }
  normalize_settings();
}

#if defined(PBL_COLOR)
static GColor blend(GColor from, GColor to, uint8_t alpha) {
  // Blend in RGB before rounding to Pebble's four levels per channel.
  GColor result = GColorBlack;
  result.r = (from.r * (255-alpha) + to.r * alpha + 127) / 255;
  result.g = (from.g * (255-alpha) + to.g * alpha + 127) / 255;
  result.b = (from.b * (255-alpha) + to.b * alpha + 127) / 255;
  return result;
}

static void recolor(GBitmap *bitmap) {
  if (!bitmap) {
    return;
  }
  GColor *palette = gbitmap_get_palette(bitmap);
  if (!palette) {
    return;
  }
  const GColor background = {.argb = s_settings.background};
  const GColor glow = {.argb = s_settings.glow};
  const GColor digit = {.argb = s_settings.digit};
  const uint8_t glow_alpha[] = {42, 72, 106, 143, 181, 219, 255};
  palette[0] = background;
  for (int i = 0; i < 7; ++i) {
    palette[i+1] = blend(background, glow, glow_alpha[i]);
  }
  for (int i = 0; i < 8; ++i) {
    palette[i+8] = blend(glow, digit, i == 7 ? 255 : (i+1)*32);
  }
}
#endif

static void update_time(struct tm *tick_time) {
  int hour = tick_time->tm_hour;
  if (!clock_is_24h_style()) {
    hour = hour % 12;
    if (!hour) {
      hour = 12;
    }
  }
  const int digits[] = {hour / 10, hour % 10,
                        tick_time->tm_min / 10, tick_time->tm_min % 10};
  // Keep only the four visible glyphs in RAM, and reload only changed digits.
  for (int i = 0; i < 4; ++i) {
    if (s_loaded[i] == digits[i]) {
      continue;
    }
    gbitmap_destroy(s_digits[i]);
    s_digits[i] = NULL;
    s_loaded[i] = -1;
    // Raw PNG resources preserve the coverage palette's exact index order.
    // Monochrome resources are explicitly compiled as legacy 1Bit for inversion.
    s_digits[i] = gbitmap_create_with_resource(DIGIT_RESOURCES[digits[i]]);
#if defined(PBL_COLOR)
    recolor(s_digits[i]);
#endif
    if (s_digits[i]) {
      s_loaded[i] = digits[i];
    } else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Could not load digit %d", digits[i]);
    }
  }
  if (s_face) {
    layer_mark_dirty(s_face);
  }
}

static void draw_face(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, (GColor){.argb = s_settings.background});
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
#if defined(PBL_COLOR)
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
#else
  graphics_context_set_compositing_mode(ctx,
      s_settings.digit == GColorWhite.argb ? GCompOpAssign : GCompOpAssignInverted);
#endif
  // Native resource sizes, with positions derived from the actual framebuffer.
  const int16_t column_gap = bounds.size.w >= 200 ? 8 : 6;
  const int16_t row_gap = bounds.size.h >= 200 ? 6 : 4;
  for (int i = 0; i < 4; ++i) {
    if (!s_digits[i]) {
      continue;
    }
    const GSize size = gbitmap_get_bounds(s_digits[i]).size;
    const int16_t x = bounds.origin.x + (bounds.size.w - 2*size.w - column_gap)/2
                      + (i % 2) * (size.w + column_gap);
    const int16_t y = bounds.origin.y + (bounds.size.h - 2*size.h - row_gap)/2
                      + (i / 2) * (size.h + row_gap);
    graphics_draw_bitmap_in_rect(ctx, s_digits[i], GRect(x, y, size.w, size.h));
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static bool receive_color(DictionaryIterator *iter, uint32_t key, uint8_t *color) {
  Tuple *tuple = dict_find(iter, key);
  if (!tuple || (tuple->type != TUPLE_UINT && tuple->type != TUPLE_INT) ||
      tuple->length != sizeof(uint32_t) || tuple->value->uint32 > 0xffffff) {
    return false;
  }
  const uint8_t value = GColorFromHEX(tuple->value->uint32).argb;
  const bool changed = value != *color;
  *color = value;
  return changed;
}

static void inbox_received(DictionaryIterator *iter, void *context) {
  bool changed = receive_color(iter, MESSAGE_KEY_DIGIT_COLOR, &s_settings.digit);
  changed |= receive_color(iter, MESSAGE_KEY_BACKGROUND_COLOR, &s_settings.background);
#if defined(PBL_COLOR)
  changed |= receive_color(iter, MESSAGE_KEY_GLOW_COLOR, &s_settings.glow);
#endif
  if (!changed) {
    return;
  }
  normalize_settings();
  if (persist_write_data(SETTINGS_KEY, &s_settings, sizeof(s_settings)) < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not save settings");
  }
#if defined(PBL_COLOR)
  for (int i = 0; i < 4; ++i) {
    recolor(s_digits[i]);
  }
#endif
  if (s_face) {
    layer_mark_dirty(s_face);
  }
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  s_face = layer_create(layer_get_bounds(root));
  if (!s_face) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not create watchface layer");
    return;
  }
  layer_set_update_proc(s_face, draw_face);
  layer_add_child(root, s_face);
  time_t now = time(NULL);
  update_time(localtime(&now));
}

static void window_unload(Window *window) {
  layer_destroy(s_face);
  s_face = NULL;
  for (int i = 0; i < 4; ++i) {
    gbitmap_destroy(s_digits[i]);
    s_digits[i] = NULL;
    s_loaded[i] = -1;
  }
}

static void init(void) {
  load_settings();
  s_window = window_create();
  if (!s_window) {
    return;
  }
  window_set_background_color(s_window, (GColor){.argb = s_settings.background});
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load, .unload = window_unload
  });
  window_stack_push(s_window, true);
  app_message_register_inbox_received(inbox_received);
  app_message_open(64, 16);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
