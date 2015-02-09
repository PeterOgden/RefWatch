#include <pebble.h>
#include "AppConfig.h"

#define CONFIG_VERSION 1
#define CONFIG_KEY 100
  
AppConfig app_config;

static void app_config_default() {
  app_config.version = CONFIG_VERSION;
  app_config.game_clock = 15 * 60;
  app_config.play_clock = 25;
  app_config.timeouts = 3;
  app_config.periods = 4;
  app_config.post_snap = 0;
}

void app_config_init() {
  if (persist_exists(CONFIG_KEY))
  {
    if (persist_read_data(CONFIG_KEY, &app_config, sizeof(AppConfig)) == sizeof(AppConfig)) {
      return;
    }
  }
  app_config_default();
}

#define GAME_CLOCK 1
#define PLAY_CLOCK 2
#define TIMEOUTS 3
#define PERIODS 4
#define POST_SNAP 5

void app_config_reload(DictionaryIterator* iterator) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Starting reload");
  app_config_default();
  Tuple* t = dict_read_first(iterator);
  while (t != NULL) {
    switch(t->key) {
    case GAME_CLOCK: app_config.game_clock = t->value->uint16; break;
    case PLAY_CLOCK: app_config.play_clock = t->value->uint8; break;
    case TIMEOUTS: app_config.timeouts = t->value->uint8; break;
    case PERIODS: app_config.periods = t->value->uint8; break;
    case POST_SNAP: app_config.post_snap = t->value->uint8; break;
    default: break;
    }
    t = dict_read_next(iterator);
  }
  persist_write_data(CONFIG_KEY, &app_config, sizeof(AppConfig));
}