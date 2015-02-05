#include <pebble.h>
#include "ChoiceLayer.h"

struct ChoiceLayer_t {
  Layer* layer;
  void* context;
  ChoiceLayerCallback callback;
  const char** choices; 
};

static void draw_triangle(GContext* ctx, GPoint p0, GPoint p1, GPoint p2) {
  graphics_draw_line(ctx, p0, p1);
  graphics_draw_line(ctx, p1, p2);
  graphics_draw_line(ctx, p2, p0);
}

static void draw_item(GContext* ctx, uint8_t y_offset, uint8_t height, const char* text) {
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  uint8_t font_y = (height - 24) / 2 + y_offset;
  graphics_draw_text(ctx, text, font, (GRect){
    .origin = {.x = 10, .y = font_y - 4}, .size = {.w = 100, .h = 24}
  }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  draw_triangle(ctx,
               (GPoint){.x = 120, .y = font_y},
               (GPoint){.x = 120, .y = font_y + 24},
               (GPoint){.x = 132, .y = font_y + 12});
  
}

static void choicelayer_draw(Layer* layer, GContext* ctx) {
  
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  GRect bounds = layer_get_bounds(layer);
  uint8_t spacing = bounds.size.h / 3;
  
  ChoiceLayer* cl = *(ChoiceLayer**)layer_get_data(layer);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Begin Choice Draw");
  APP_LOG(APP_LOG_LEVEL_DEBUG, "First item: %s", cl->choices[0]);
  draw_item(ctx, 0, spacing, cl->choices[0]);
  draw_item(ctx, spacing, spacing, cl->choices[1]);
  draw_item(ctx, spacing * 2, spacing, cl->choices[2]);
  
  graphics_draw_line(ctx, (GPoint){.x = 0, .y = spacing}, (GPoint){.x = bounds.size.w, .y = spacing});
  graphics_draw_line(ctx, (GPoint){.x = 0, .y = spacing * 2}, (GPoint){.x = bounds.size.w, .y = spacing * 2});
}

static void up_click(ClickRecognizerRef re, void* context) {
  ChoiceLayer* l = (ChoiceLayer*)context;
  l->callback(l->context, 0);
}

static void middle_click(ClickRecognizerRef re, void* context) {
  ChoiceLayer* l = (ChoiceLayer*)context;
  l->callback(l->context, 1);
}

static void down_click(ClickRecognizerRef re, void* context) {
  ChoiceLayer* l = (ChoiceLayer*)context;
  l->callback(l->context, 2);
}

static void choicelayer_clickconfig(void* context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click);
  window_single_click_subscribe(BUTTON_ID_SELECT, middle_click);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click);
}

ChoiceLayer* choicelayer_create(GRect rect) {
  ChoiceLayer* ret = (ChoiceLayer*)calloc(1, sizeof(ChoiceLayer));
  if (!ret) return ret;
  ret->layer = layer_create_with_data(rect, sizeof(void*));
  layer_set_update_proc(ret->layer, choicelayer_draw);
  void** data = layer_get_data(ret->layer);
  *data = ret;
  return ret;
}

ChoiceLayer* choicelayer_create_from_window(Window* window) {
  Layer* root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  ChoiceLayer* ret = choicelayer_create(bounds);
  if (!ret) return ret;
  choicelayer_set_window(ret, window);
  return ret;
}

void choicelayer_destroy(ChoiceLayer* layer) {
  if (layer) {
    layer_destroy(layer->layer);
    free(layer);
  }
}

Layer* choicelayer_get_layer(ChoiceLayer* layer) {
  return layer->layer;
}

void choicelayer_set_choices(ChoiceLayer* layer, const char** choices) {
  layer->choices = choices;
  layer_mark_dirty(choicelayer_get_layer(layer));
}

void choicelayer_set_callback(ChoiceLayer* layer, ChoiceLayerCallback callback, void* context) {
  layer->callback = callback;
  layer->context = context;
}

void choicelayer_set_window(ChoiceLayer* layer, Window* window) {
  window_set_click_config_provider_with_context(window, choicelayer_clickconfig, layer);
}



