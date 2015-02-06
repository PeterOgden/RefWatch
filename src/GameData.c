#include <pebble.h>
#include "GameData.h"

void game_data_init(GameData* data) {
  game_list_init(&data->home.scores);
  game_list_init(&data->away.scores);
  game_list_init(&data->home.penalties);
  game_list_init(&data->away.penalties);
}
void game_data_free(GameData* data) {
  if (data->timer_callbacks.timer) {
    app_timer_cancel(data->timer_callbacks.timer);
    data->timer_callbacks.timer = NULL;
  }
  game_list_free(&data->home.scores);
  game_list_free(&data->away.scores);
  game_list_free(&data->home.penalties);
  game_list_free(&data->away.penalties);
}
void game_data_reset(GameData* data) {
  game_list_clear(&data->home.scores);
  game_list_clear(&data->away.scores);
  game_list_clear(&data->home.penalties);
  game_list_clear(&data->away.penalties);
  data->home.total = 0;
  data->away.total = 0;
  data->home.timeouts = 3;
  data->away.timeouts = 3;
  data->quarter = 0;
  data->try_active = false;
  data->home_team_active = false;
  game_data_timer_reset(data);
}

static const uint32_t STORAGE_VERSION = 3;

typedef struct GameDataStorage_t {
  uint32_t version;
  Timer timer;
  uint8_t home_timeouts;
  uint8_t away_timeouts;
  uint8_t quarter;
  uint8_t try_active;
  uint8_t home_team_active;
} GameDataStorage;

bool game_data_read(GameData* data, uint32_t key) {
  if (!persist_exists(key)) return false;
  GameDataStorage storage;
  if (persist_read_data(key, &storage, sizeof(storage)) != sizeof(storage)) return false;
  if (storage.version != STORAGE_VERSION) return false;
  
  data->timer = storage.timer;
  data->home.timeouts = storage.home_timeouts;
  data->away.timeouts = storage.away_timeouts;
  data->quarter = storage.quarter;
  data->try_active = storage.try_active;
  data->home_team_active = storage.home_team_active;
  game_list_read(&data->home.scores, key + 1);
  game_list_read(&data->away.scores, key + 2);
  game_list_read(&data->home.penalties, key + 3);
  game_list_read(&data->away.penalties, key + 4);
  data->home.total = game_list_total_score(&data->home.scores);
  data->away.total = game_list_total_score(&data->away.scores);
  
  if (data->timer.running) {
    if (game_data_timer_get_value(data) == 0.0f) {
      game_data_timer_stop(data);
    } else {
      game_data_timer_start(data);
    }
  } else {
    game_data_timer_stop(data);
  }
  return true;
}
void game_data_write(GameData* data, uint32_t key) {
  GameDataStorage storage = {
    STORAGE_VERSION,
    data->timer,
    data->home.timeouts,
    data->away.timeouts,
    data->quarter,
    data->try_active,
    data->home_team_active
  };
  persist_write_data(key, &storage, sizeof(storage));
  game_list_write(&data->home.scores, key + 1);
  game_list_write(&data->away.scores, key + 2);
  game_list_write(&data->home.penalties, key + 3);
  game_list_write(&data->away.penalties, key + 4);
}
static double time_double() {
  time_t seconds;
  uint16_t milliseconds;
  time_ms(&seconds, &milliseconds);
  return seconds + (double)milliseconds/1000.0;
}

static void timer_handle(void* ctx) {
  GameData* data = (GameData*)ctx;
  data->timer_callbacks.timer = app_timer_register(100, timer_handle, ctx);
  if (data->timer_callbacks.on_tick) data->timer_callbacks.on_tick(NULL);
  if (game_data_timer_get_value(data) == 0.0) {
    if (data->timer_callbacks.on_expire) data->timer_callbacks.on_expire(NULL);
    game_data_timer_stop(data);
  }
}

void game_data_timer_start(GameData* data) {
  if (!data->timer.running) {
    data->timer.started = time_double();
    data->timer.running = true;
  }
  if (!data->timer_callbacks.timer) {
    data->timer_callbacks.timer = app_timer_register(100, timer_handle, data);
  }
  if (data->timer_callbacks.on_start) data->timer_callbacks.on_start(NULL);
}
void game_data_timer_stop(GameData* data) {
  if (data->timer.running) {
    data->timer.initial = game_data_timer_get_value(data);
    data->timer.running = false;
  }
  if (data->timer_callbacks.timer) {
    app_timer_cancel(data->timer_callbacks.timer);
    data->timer_callbacks.timer = NULL;
  }
  if (data->timer_callbacks.on_stop) data->timer_callbacks.on_stop(NULL);
}
void game_data_timer_reset(GameData* data) {
  if (game_data_timer_is_running(data)) game_data_timer_stop(data);
  data->timer.initial = data->timer.reset_to;
  if (data->timer_callbacks.on_tick) data->timer_callbacks.on_tick(NULL);
}

void game_data_timer_set_reset(GameData* data, uint16_t value) {
  data->timer.reset_to = value;
}

double game_data_timer_get_value(GameData* data) {
  if (data->timer.running) {
    double current = time_double();
    double total_seconds = data->timer.started - current + data->timer.initial;
    if (total_seconds < 0) total_seconds = 0.0;
    return total_seconds;
  } else {
    return (uint16_t) data->timer.initial;
  }
}

void game_data_timer_set_callbacks(GameData* data, 
          TimerCallback start, TimerCallback stop, TimerCallback tick, TimerCallback expire) {
  data->timer_callbacks.on_start = start;
  data->timer_callbacks.on_stop = stop;
  data->timer_callbacks.on_tick = tick;
  data->timer_callbacks.on_expire = expire;
  
  if (data->timer.running) {
    if (data->timer_callbacks.on_start) data->timer_callbacks.on_start(NULL);
  } else {
    if (data->timer_callbacks.on_stop) data->timer_callbacks.on_stop(NULL);
  }
  if (data->timer_callbacks.on_tick) data->timer_callbacks.on_tick(NULL);
}

bool game_data_timer_is_running(GameData* data) {
  return data->timer.running;
}

void team_data_new_score(TeamData* data, uint8_t score, uint8_t quarter, uint8_t time) {
  data->total += score;
  game_list_add(&data->scores, score, quarter, time);
}
void team_data_add_pat(TeamData* data, uint8_t score) {
  data->total += score;
  game_list_amend_last(&data->scores, score + 6);
}