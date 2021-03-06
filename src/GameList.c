#include <pebble.h>
#include "GameList.h"
#include "AppConfig.h"
static const int BYTES_PER_ENTRY = 2;
  
void game_list_init(GameList* list) {
  list->data = calloc(2, 1);
  list->size = 0;
  list->capacity = 1;
}

void game_list_free(GameList* list) {
  free(list->data);
}

uint16_t game_list_size(GameList* list) {
  return list->size;
}

bool game_list_empty(GameList* list) {
  return list->size == 0;
}

static void game_list_grow(GameList* list, uint16_t size) {
  uint16_t new_cap = list->capacity;
  if (list->capacity == 0) new_cap = 1;
  while (new_cap <= size) new_cap <<= 1;
  list->data = realloc(list->data, new_cap * BYTES_PER_ENTRY);
  list->capacity = new_cap;
}

static void game_list_set_size(GameList* list, uint16_t size) {
  if (size > list->capacity) game_list_grow(list, size);
  list->size = size;
}

uint16_t game_list_total_score(GameList* list) {
  uint16_t score = 0;
  for (int i = 0; i < list->size; ++i) {
    score += list->data[i] & 0x00FF;
  }
  return score;
}

void game_list_add(GameList* list, uint8_t value, uint8_t quarter, uint16_t time) {
  if (list->size == list->capacity) {
    game_list_grow(list, list->size + 1);
  }
  list->data[list->size++] = (quarter << 8) | value;
}

void game_list_amend_last(GameList* list, uint8_t score) {
  list->data[list->size - 1] &= 0xFF00;
  list->data[list->size - 1] |= score;
}

void game_list_clear(GameList* list) {
  list->size = 0;
}

static const char* ordinals[] = {"1st", "2nd", "3rd", "4th", "Overtime"};
const char* quarter_to_text(uint8_t quarter) {
  if (quarter < app_config.periods) return ordinals[quarter];
  else return ordinals[4];
}

void game_list_text(GameList* list, uint8_t index, char* buffer, uint16_t size) {
  uint8_t score = list->data[index] & 0x00FF;
  uint8_t quarter = list->data[index] >> 8;
  snprintf(buffer, size, "%s - %d", quarter_to_text(quarter), score);
}

void game_list_write(GameList* list, uint32_t key) {
  persist_write_data(key, list->data, list->size * 2);
}

void game_list_read(GameList* list, uint32_t key) {
  int size = persist_get_size(key);
  if (size == E_DOES_NOT_EXIST) return;
  game_list_set_size(list, (uint16_t)size / 2);
  persist_read_data(key, list->data, size);
}