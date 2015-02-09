#pragma once
#include "GameList.h"

typedef struct TeamData_t {
  GameList scores;
  GameList penalties;
  uint16_t total;
  uint8_t timeouts;
} TeamData;
  
typedef struct Timer_t {
  double initial;
  double started;
  bool running;
  uint16_t reset_to;
} Timer;

typedef void (*TimerCallback)(void*);
typedef struct TimerInternal_t {
  TimerCallback on_start;
  TimerCallback on_stop;
  TimerCallback on_tick;
  TimerCallback on_expire;
  AppTimer* timer;
} TimerInternal;

typedef struct GameData_t {
  // Team information
  TeamData home;
  TeamData away;
  
  // Runtime information
  bool home_team_active;
  bool try_active;
  
  // Timer information
  uint8_t quarter;
  
  Timer timer;
  TimerInternal timer_callbacks;
  
  uint8_t play_clock;
  
} GameData;

static const int SCORE_OFFSET = offsetof(TeamData, scores);
static const int PENALTY_OFFSET = offsetof(TeamData, penalties);

void game_data_init(GameData* data);
void game_data_free(GameData* data);
void game_data_reset(GameData* data);

bool game_data_read(GameData* data, uint32_t key);
void game_data_write(GameData* data, uint32_t key);

void game_data_timer_start(GameData* data);
void game_data_timer_stop(GameData* data);
void game_data_timer_reset(GameData* data);

void game_data_timer_set_reset(GameData* data, uint16_t value);
double game_data_timer_get_value(GameData* data);
void game_data_timer_set_callbacks(GameData* data, 
          TimerCallback start, TimerCallback stop, TimerCallback tick, TimerCallback expire);
bool game_data_timer_is_running(GameData* data);

void team_data_new_score(TeamData* data, uint8_t score, uint8_t quarter, uint8_t time);
void team_data_add_pat(TeamData* data, uint8_t score);

void team_data_add_penalty(TeamData* data, uint8_t number, uint8_t quarter, uint8_t time);
void team_data_add_timeout(TeamData* data, uint8_t number, uint8_t quarter, uint8_t time);

