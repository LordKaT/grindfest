#ifndef UI_H
#define UI_H

#include "game.h"

// Init/Cleanup ncurses
// Init/Cleanup ncurses
void ui_init(void);
void ui_cleanup(void);

typedef enum {
    UI_LAYOUT_GAME,    // 54x16 Map, 26x24 Panel
    UI_LAYOUT_CREATOR  // 40x16 Map, 40x24 Panel
} UILayout;

void ui_set_layout(UILayout layout);

// Rendering
void ui_clear(void);
void ui_render_map(Map* map, const Entity* player, const Entity entities[], int entity_count, RenderMode mode);
void ui_render_stats(const Entity* player);
void ui_render_log(void);
void ui_render_input_line(const char* current_input);
void ui_refresh(void);

// Menu
void ui_open_menu(void);
void ui_close_menu(void);
void ui_render_menu(const Entity* player);
void ui_render_creator_menu(const char* title, const char** items, int count, int selection, const char* description);

// Input
// Non-blocking check for key? Or blocking 'getch'? 
// We likely need a blocking call that returns a Key or String.
// However, since we have a command line, we need to handle character appending.
// For the scaffold, we might return a simpler key code or update a buffer.
int ui_get_input(char* input_buffer, int max_len);

// Blocking string input at bottom line
void ui_get_string(const char* prompt, char* buffer, int max_len);

// Logging
void ui_log(const char* fmt, ...);

#endif
