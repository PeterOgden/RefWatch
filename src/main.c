#include <pebble.h>
#include "GameData.h"

static GameData game_data;

static Window *s_main_window;
static Layer *s_static_layer;
static Layer *s_score_layer;
static TextLayer *s_time_layer;

static Window* s_menu_window;
static MenuLayer* s_menu_layer;

static void draw_static(Layer* layer, GContext* ctx) {
  GFont team_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  graphics_context_set_text_color(ctx, GColorBlack);
  GRect bounds = layer_get_bounds(layer); 
  graphics_draw_text(ctx, "AWAY", team_font, (GRect){
      .origin = {.x = 0, . y = 0}, .size = {.h = bounds.size.h, .w = bounds.size.w / 2}
  }, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, "HOME", team_font, (GRect){
      .origin = {.x = bounds.size.w / 2, . y = 0}, .size = {.h = bounds.size.h, .w = bounds.size.w / 2}
  }, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void draw_team_data(Layer* layer, GContext* ctx, TeamData* data, int x_offset, int width) {
  GFont score_font = fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
  char buffer[4];
  
  snprintf(buffer, 4, "%d", data->total);
  graphics_draw_text(ctx, buffer, score_font, (GRect){
      .origin = {.x = x_offset, . y = -3}, .size = {.h = 28, .w = width}
  }, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  
  for (int i = 0; i < data->timeouts; ++i) {
    graphics_fill_rect(ctx, (GRect) {
      .origin = {.x = x_offset + i * 15 + 13, .y = 38}, .size = {.w = 13, .h = 4}
    }, 0, GCornerNone);
  }
  
}

static void draw_game_data(Layer* layer, GContext* ctx) {  
  GFont quarter_font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  GRect bounds = layer_get_bounds(layer);
  
  int x_offset = 0;
  int width = bounds.size.w / 2;

  draw_team_data(layer, ctx, &game_data.away, x_offset, width);
  draw_team_data(layer, ctx, &game_data.home, width, width);
  
  graphics_draw_text(ctx, quarter_to_text(game_data.quarter), quarter_font, (GRect) {
    .origin = {.x = 0, .y = 40}, .size = {.w = bounds.size.w, .h = 24}
  }, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void start_timer();
static void stop_timer();

static void update_time(void* ctx) {
  static char buffer[10];
  uint16_t total_seconds = game_data_timer_get_value(&game_data);
  snprintf(buffer, 10, "%02d:%02d", total_seconds / 60, total_seconds % 60);
  text_layer_set_text(s_time_layer, buffer);
}

static void update_display() {
  layer_mark_dirty(s_score_layer);
}

static void on_start(void* ctx) {
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorBlack);
}

static void on_stop(void* ctx) {
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_background_color(s_time_layer, GColorWhite);
}

static void on_expire(void* ctx) {
  vibes_long_pulse();
}

static void start_timer() {
  game_data_timer_start(&game_data);
}

static void stop_timer() {
  game_data_timer_stop(&game_data);
}

static void main_window_load(Window *window) {
  Layer* root_layer = window_get_root_layer(window);
  // Create static layer
  s_static_layer = layer_create((GRect){
    .origin = {.x = 5, .y = 5}, .size = {.w = 134, .h = 15}
  });
  layer_set_update_proc(s_static_layer, draw_static);
  // Add it as a child layer to the Window's root layer
  layer_add_child(root_layer, s_static_layer);
  layer_mark_dirty(s_static_layer);
  
  s_score_layer = layer_create((GRect) {
    .origin = {.x = 5, .y = 20}, .size = {.w = 134, .h = 70}
  });
  layer_set_update_proc(s_score_layer, draw_game_data);
  layer_add_child(root_layer, s_score_layer);
  
  update_display();
  
  s_time_layer = text_layer_create((GRect){
    .origin = {.x = 5, .y = 90}, .size = {.h = 58, .w = 134}
  });
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_time_layer));
  game_data_timer_set_callbacks(&game_data, on_start, on_stop, update_time, on_expire);
  update_time(NULL);
}

static void main_window_unload(Window *window) {
  // Destroy Layers
  layer_destroy(s_static_layer);
  layer_destroy(s_score_layer);
  text_layer_destroy(s_time_layer);
}

typedef void (*MenuCallback)(int index);
static void reset_menu(const char** text, int number, MenuCallback callback);

static const char* main_scores_items[] = {"Touchdown", "Field Goal", "Safety"};
static const char* try_scores_items[] = {"2-point", "1-point", "Failed"};
static const char* team_items[] = {"Home Team", "Away Team"};

static const char* main_menu_items[] = {"Timeout", "End Quarter", "View Scores", "Reset Game"};
static const char* time_menu_items[] = {"Game Clock", "Play Clock", "Timeout", "Half"};


static const char** current_menu_text = try_scores_items;
static int current_menu_number = 3;
static MenuCallback current_menu_callback;


static void main_score(int index) {
  uint8_t points = 0;
  switch (index) {
    case 0: points = 6; break;
    case 1: points = 3; break;
    case 2: points = 2; break;
  }
  team_data_new_score(game_data.home_team_active?&game_data.home:&game_data.away, points, game_data.quarter);
  if (points == 6) game_data.try_active = true;
  update_display();
  window_stack_pop(false);
}

static void set_team_score(int index) {
  game_data.home_team_active = index == 0;
  current_menu_text = main_scores_items;
  current_menu_number = 3;
  current_menu_callback = main_score;
  reset_menu(main_scores_items, 3, main_score);
}

static void set_team_timeout(int index) {
  switch (index) {
    case 0: game_data.home.timeouts--; break;
    case 1: game_data.away.timeouts--; break;
  }
  window_stack_pop(false);
  update_display();
}

static void try_score(int index) {
  uint8_t points = 0;
  switch (index) {
    case 0: points = 2; break;
    case 1: points = 1; break;
    case 2: points = 0; break;
  }
  team_data_add_pat(game_data.home_team_active?&game_data.home:&game_data.away, points);
  game_data.try_active = false;
  update_display();
  window_stack_pop(false);
}

static uint16_t get_menu_rows_number(MenuLayer* layer, uint16_t section, void* data) {
  return current_menu_number;
}

static void draw_menu_row(GContext* ctx, const Layer* cell_layer, MenuIndex* index, void* data) {
  menu_cell_basic_draw(ctx, cell_layer, current_menu_text[index->row], NULL, NULL);
}

static void menu_click(MenuLayer* layer, MenuIndex* index, void* data) {
  current_menu_callback(index->row);
}

static uint16_t games_list_menu_rows_number(MenuLayer* layer, uint16_t section, void* data) {
  return game_list_size((GameList*)data);
}

static void games_list_draw_menu_row(GContext* ctx, const Layer* cell_layer, MenuIndex* index, void* data) {
  static char buffer[10];
  game_list_text((GameList*)data, index->row, buffer, 10);
  menu_cell_basic_draw(ctx, cell_layer, buffer, NULL, NULL);
}

static void games_list_menu_click(MenuLayer* layer, MenuIndex* index, void* data) {
  window_stack_pop(false);
}



static void reset_menu(const char** text, int number, MenuCallback callback) {
  current_menu_text = text;
  current_menu_number = number;
  current_menu_callback = callback;
  if (window_is_loaded(s_menu_window)) {
    menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = get_menu_rows_number,
      .draw_row = draw_menu_row,
      .select_click = menu_click
    });
    MenuIndex index;
    index.row = 0;
    index.section = 0;
    menu_layer_set_selected_index(s_menu_layer, index, MenuRowAlignTop, false);
  }
}

static void set_game_list_menu(GameList* list) {
  if (window_is_loaded(s_menu_window)) {
    menu_layer_set_callbacks(s_menu_layer, list, (MenuLayerCallbacks) {
      .get_num_rows = games_list_menu_rows_number,
      .draw_row = games_list_draw_menu_row,
      .select_click = games_list_menu_click
    });
  }
}

static void show_team_score(int index) {
  if (index == 0) set_game_list_menu(&game_data.home.scores);
  else set_game_list_menu(&game_data.away.scores);
}


static void main_menu_click(int index) {
  switch (index) {
    case 0: reset_menu(team_items, 2, set_team_timeout); break;
    case 1: 
      if (game_data.quarter++ == 1) {
        game_data.home.timeouts = 3;
        game_data.away.timeouts = 3;
      }
      window_stack_pop(false);
      break;
    case 2: reset_menu(team_items, 2, show_team_score); break;
    case 3: game_data_reset(&game_data); update_display(); window_stack_pop(false); break;
  }
}

static void time_menu_click(int index) {
  int seconds = 0;
  switch (index) {
  case 0: seconds = 15*60; break;
  case 1: seconds = 25; break;
  case 2: seconds = 90; break;
  case 3: seconds = 60 * 20; break;
  }
  game_data_timer_set_reset(&game_data, seconds);
  game_data_timer_reset(&game_data);
  window_stack_pop(false);
}

static void menu_window_load(Window* window) {
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  s_menu_layer = menu_layer_create(bounds);

  menu_layer_set_click_config_onto_window(s_menu_layer, s_menu_window);
  // Needed to avoid some potential messyness
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = get_menu_rows_number,
    .draw_row = draw_menu_row,
    .select_click = menu_click
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void menu_window_unload(Window* window) {
  menu_layer_destroy(s_menu_layer);
}

static void up_click(ClickRecognizerRef re, void* ctx) {
  if (game_data.try_active) {
    reset_menu(try_scores_items, 3, try_score);
  } else {
    reset_menu(team_items, 2, set_team_score);
  }
  if (window_is_loaded(s_menu_window)) menu_layer_reload_data(s_menu_layer);
  window_stack_push(s_menu_window, true);
}

static void middle_click(ClickRecognizerRef re, void* ctx) {
  reset_menu(main_menu_items, 4, main_menu_click);
  window_stack_push(s_menu_window, true);
}

static void down_long(ClickRecognizerRef re, void* ctx) {
  reset_menu(time_menu_items, 4, time_menu_click);
  window_stack_push(s_menu_window, true);
}

static void down_click(ClickRecognizerRef re, void* ctx) {
  if (game_data_timer_is_running(&game_data)) stop_timer();
  else start_timer();
}

static void configure_click(void* ctx) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click);
  window_single_click_subscribe(BUTTON_ID_SELECT, middle_click);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click);
  window_long_click_subscribe(BUTTON_ID_DOWN, 1000, down_long, NULL);
}
static const int GAME_DATA_KEY = 0;
static const int HOME_SCORE_KEY = 1;
static const int AWAY_SCORE_KEY = 2;

static void init() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "In init");
  // Create the score vectors
  game_data_init(&game_data);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Game list init");
  // Load data from persistent storage
  if (!game_data_read(&game_data, GAME_DATA_KEY)) {
    game_data_reset(&game_data);
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating windows");
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  s_menu_window = window_create();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Set Handlers");
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_set_window_handlers(s_menu_window, (WindowHandlers) {
    .load = menu_window_load,
    .unload = menu_window_unload
  });
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Set click providers");
  window_set_click_config_provider(s_main_window, configure_click);
  // Show the Window on the watch, with animated=true
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Push main window");
  window_stack_push(s_main_window, true);
 
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  game_data_write(&game_data, GAME_DATA_KEY);
  
  // Free the score list
  game_data_free(&game_data);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
