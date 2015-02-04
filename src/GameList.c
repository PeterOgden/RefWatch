#include <pebble.h>
#include "GameList.h"

void game_list_init(GameList* list) {
  list->data = malloc(1);
  list->size = 0;
  list->capacity = 1;
}

void game_list_free(GameList* list) {
  free(list->data);
}

uint16_t game_list_size(GameList* list) {
  return list->size;
}

static void game_list_set_size(GameList* list, uint16_t size) {
  uint16_t new_cap = list->capacity;
  if (list->capacity == 0) new_cap = 1;
  while (new_cap <= size) new_cap <<= 1;
  list->data = realloc(list->data, new_cap);
  list->size = size;
}

uint16_t game_list_total_score(GameList* list) {
  uint16_t score = 0;
  for (int i = 0; i < list->size; ++i) {
    score += list->data[i] & 0x0F;
  }
  return score;
}

void game_list_add(GameList* list, uint8_t score, uint8_t quarter) {
  if (list->size == list->capacity) {
    uint16_t new_size = 1;
    if (list->capacity != 0) new_size = list->capacity << 1;
    list->data = realloc(list->data, new_size);
  }
  list->data[list->size++] = (quarter << 4) | score;
}

void game_list_amend_last(GameList* list, uint8_t score) {
  list->data[list->size - 1] &= 0xF0;
  list->data[list->size - 1] |= score;
}

void game_list_clear(GameList* list) {
  list->size = 0;
}

static const char* ordinals[] = {"1st", "2nd", "3rd", "4th", "Overtime"};
const char* quarter_to_text(uint8_t quarter) {
  if (quarter < 4) return ordinals[quarter];
  else return ordinals[4];
}

void game_list_text(GameList* list, uint8_t index, char* buffer, uint16_t size) {
  uint8_t score = list->data[index] & 0x0F;
  uint8_t quarter = list->data[index] >> 4;
  snprintf(buffer, size, "%s - %d", quarter_to_text(quarter), score);
}

void game_list_write(GameList* list, uint32_t key) {
  persist_write_data(key, list->data, list->size);
}

void game_list_read(GameList* list, uint32_t key) {
  int size = persist_get_size(key);
  if (size == E_DOES_NOT_EXIST) return;
  game_list_set_size(list, (uint16_t)size);
  persist_read_data(key, list->data, size);
}