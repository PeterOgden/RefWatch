#pragma once
#include <pebble.h>

struct ChoiceLayer_t;
typedef struct ChoiceLayer_t ChoiceLayer;
typedef void (*ChoiceLayerCallback)(void* data, int index);

// Create new layer
ChoiceLayer* choicelayer_create(GRect bounds);
ChoiceLayer* choicelayer_create_from_window(Window* window);
void choicelayer_destroy(ChoiceLayer* layer);

// get the underlying layer - this must be done
Layer* choicelayer_get_layer(ChoiceLayer* layer);
void choicelayer_set_choices(ChoiceLayer* layer, const char** choices);
void choicelayer_set_callback(ChoiceLayer* layer, ChoiceLayerCallback callback, void* data);
void choicelayer_set_window(ChoiceLayer* layer, Window* window);
