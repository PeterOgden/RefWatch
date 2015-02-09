#pragma once
#include <pebble.h>
  
typedef struct AppConfig_t {
  uint16_t version;
  uint16_t game_clock;
  uint8_t play_clock;
  uint8_t timeouts;
  uint8_t periods;
  uint8_t post_snap;
} AppConfig;

extern AppConfig app_config;

void app_config_init();
void app_config_reload(DictionaryIterator* iterator);