#pragma once
#include <pebble.h>
  
typedef struct GameList_t {
  uint16_t* data;
  uint16_t size;
  uint16_t capacity;
} GameList;

// Creation and deletion
void game_list_init(GameList* list);
void game_list_free(GameList* list);

uint16_t game_list_size(GameList* list);
bool game_list_empty(GameList* list);
void game_list_clear(GameList* list);
void game_list_add(GameList* list, uint8_t value, uint8_t quarter, uint16_t time);
void game_list_amend_last(GameList* list, uint8_t value);
uint16_t game_list_total_score(GameList* list);
void game_list_text(GameList* list, uint8_t index, char* buffer, uint16_t size);

void game_list_write(GameList* list, uint32_t key);
void game_list_read(GameList* list, uint32_t key);

const char* quarter_to_text(uint8_t quarter);