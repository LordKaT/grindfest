#define _XOPEN_SOURCE_EXTENDED 1
#include <ncurses.h>
#include <locale.h>
#include <stdarg.h>
#include <string.h>
#include "ui.h"
#include "turn.h"

// Layout Definitions
#define MAP_VIEW_WIDTH 54
#define MAP_VIEW_HEIGHT 16
#define LOG_HEIGHT 7
#define INPUT_HEIGHT 1

#define PANEL_WIDTH 26
#define PANEL_HEIGHT 24

static WINDOW *win_map;
static WINDOW *win_panel;
static WINDOW *win_log;
static WINDOW *win_input;

// Simple log buffer
#define MAX_LOG_LINES 50
static char log_history[MAX_LOG_LINES][64];
//static int log_head = 0;
static int log_count = 0;

void ui_init(void) {
    setlocale(LC_ALL, ""); // Enable wide chars
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0); // Hide cursor initially

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_YELLOW, COLOR_BLACK); // Player
        init_pair(2, COLOR_WHITE, COLOR_BLACK);  // Default
        init_pair(3, COLOR_RED, COLOR_BLACK);    // Enemy
        init_pair(4, COLOR_CYAN, COLOR_BLACK);   // UI borders
        init_pair(5, COLOR_BLUE, COLOR_BLACK);   // Sound Clear
        init_pair(6, COLOR_CYAN, COLOR_BLACK);   // Sound Muffled (reuse cyan but specific semantic)
    }

    bkgd(COLOR_PAIR(2)); // Force stdscr to black
    refresh(); // Refresh stdscr

    // Create Windows
    win_map = newwin(MAP_VIEW_HEIGHT, MAP_VIEW_WIDTH, 0, 0);
    win_panel = newwin(PANEL_HEIGHT, PANEL_WIDTH, 0, MAP_VIEW_WIDTH);
    win_log = newwin(LOG_HEIGHT, MAP_VIEW_WIDTH, MAP_VIEW_HEIGHT, 0);
    win_input = newwin(INPUT_HEIGHT, MAP_VIEW_WIDTH, MAP_VIEW_HEIGHT + LOG_HEIGHT, 0);
    keypad(win_input, TRUE);

    // Force background to black for all windows
    wbkgd(win_map, COLOR_PAIR(2));
    wbkgd(win_panel, COLOR_PAIR(2));
    wbkgd(win_log, COLOR_PAIR(2));
    wbkgd(win_input, COLOR_PAIR(2));
}

void ui_cleanup(void) {
    delwin(win_map);
    delwin(win_panel);
    delwin(win_log);
    delwin(win_input);
    endwin();
}

void ui_clear(void) {
    werase(win_map);
    werase(win_panel);
    werase(win_log);
    werase(win_input);
    
    // Draw borders/separators if needed
    // Typically box() takes character space, but our layout implies tight packing.
    // We'll trust the layout for now.
}

// Wall Mask Directions
enum { DIR_N = 1, DIR_E = 2, DIR_S = 4, DIR_W = 8 };

static cchar_t* get_wall_glyph(int mask) {
    static cchar_t* table[16] = {0};
    static bool inited = false;
    if (!inited) {
        table[0] = WACS_BLOCK; // Isolated?
        table[1] = WACS_VLINE;
        table[2] = WACS_HLINE;
        table[3] = WACS_LLCORNER;
        table[4] = WACS_VLINE;
        table[5] = WACS_VLINE;
        table[6] = WACS_ULCORNER;
        table[7] = WACS_LTEE;
        table[8] = WACS_HLINE;
        table[9] = WACS_LRCORNER;
        table[10] = WACS_HLINE;
        table[11] = WACS_BTEE;
        table[12] = WACS_URCORNER;
        table[13] = WACS_RTEE;
        table[14] = WACS_TTEE;
        table[15] = WACS_PLUS;

        inited = true;
    }
    if (mask == 0)
        return table[0]; // Return consistent WACS_BLOCK instead of NULL
    return table[mask];
}

static bool tile_is_known_wall(const Map* map, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return false;
    const Tile* t = &map->tiles[x][y];
    if (!(t->visible || t->explored)) return false;
    return t->type == TILE_WALL;
}

static int wall_mask_at(const Map* map, int x, int y) {
    int m = 0;
    if (tile_is_known_wall(map, x, y-1)) m |= DIR_N;
    if (tile_is_known_wall(map, x+1, y)) m |= DIR_E;
    if (tile_is_known_wall(map, x, y+1)) m |= DIR_S;
    if (tile_is_known_wall(map, x-1, y)) m |= DIR_W;
    return m;
}

void ui_render_map(Map* map, const Entity* player, const Entity entities[], int entity_count, RenderMode mode) {
    // Basic rendering 
    scrollok(win_map, FALSE);

    // Draw Map
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            
            // Render Mode Logic
            if (mode == RENDER_MODE_SMELL) {
                 if (map->tiles[x][y].type == TILE_WALL) {
                     // Draw walls normally (can delegate to normal logic or dup)
                 } else {
                     // Floor
                     int smell = map->smell[x][y];
                     int color = 3; // Red
                     attr_t attrs = A_NORMAL;
                     
                     if (smell == 0) {
                         // Normal floor
                         wattr_set(win_map, A_NORMAL, 2, NULL);
                         mvwaddch(win_map, y, x, '.');
                         continue;
                     } 
                     
                     if (smell >= 128) attrs = A_BOLD;
                     else if (smell < 64) attrs = A_DIM;
                     
                     wattr_set(win_map, attrs, color, NULL);
                     mvwadd_wch(win_map, y, x, WACS_BLOCK);
                     continue;
                 }
            }
            else if (mode == RENDER_MODE_SOUND) {
                if (map->tiles[x][y].type == TILE_WALL) {
                    // Draw walls normally 
                } else {
                    SoundState sound = map->sound[x][y];
                    if (sound == SOUND_CLEAR) {
                        wattr_set(win_map, A_BOLD, 5, NULL); // Blue
                        mvwadd_wch(win_map, y, x, WACS_BLOCK);
                        continue;
                    } else if (sound == SOUND_MUFFLED) {
                        wattr_set(win_map, A_DIM, 6, NULL); // Cyan
                        mvwadd_wch(win_map, y, x, WACS_CKBOARD);
                        continue;
                    } else {
                         // Normal floor
                         wattr_set(win_map, A_NORMAL, 2, NULL);
                         mvwaddch(win_map, y, x, '.');
                         continue;
                    }
                }
            }

            // Normal / Fallback Rendering
            // Visibility Check
            bool visible = map->tiles[x][y].visible;
            bool explored = map->tiles[x][y].explored;
            
            // If in debug mode, do we ignore FOV? 
            // The prompt says "We need to verify these invisible layers work." 
            // Usually debug modes bypass FOV.
            if (mode != RENDER_MODE_NORMAL) {
                visible = true; 
                explored = true;
            }

            if (!visible && !explored) {
                mvwaddch(win_map, y, x, ' ');
                continue;
            }

            int color = 2; // Default
            attr_t attrs = A_NORMAL;
            
            if (!visible && explored) {
                attrs |= A_DIM; // Dim for explored but out of sight
            }

            wattr_set(win_map, attrs, color, NULL); 
            
            if (map->tiles[x][y].type == TILE_FLOOR) {
                mvwaddch(win_map, y, x, '.');
            } 
            else if (map->tiles[x][y].type == TILE_WALL) {
                int mask = wall_mask_at(map, x, y);
                cchar_t* wglyph = get_wall_glyph(mask);
                
                if (wglyph) {
                    mvwadd_wch(win_map, y, x, wglyph);
                } else {
                    // Fallback
                     mvwaddch(win_map, y, x, ' '); 
                }
            } else {
                 mvwaddch(win_map, y, x, ' '); 
            }
            // Reset for safety (though loop sets it next time)
            wattr_set(win_map, A_NORMAL, 0, NULL);
        }
    }
    
    // 2. Render Objects / NPCs / Enemies
    for (int i = 0; i < entity_count; i++) {
        if (!entities[i].is_active) continue;
        if (entities[i].id == player->id) continue;
        if (entities[i].is_burrowed) continue;
        
        // FOV check for entities
        if (!map->tiles[entities[i].x][entities[i].y].visible) continue;
        
        // Entities should also be hidden if not visible!
        
        wattron(win_map, COLOR_PAIR(entities[i].color_pair));
        mvwaddch(win_map, entities[i].y, entities[i].x, entities[i].symbol);
        wattroff(win_map, COLOR_PAIR(entities[i].color_pair));
    }
    
    // 3. Render Player (Always visible if game is running, or map->visible check)
    if (player->is_active) {
        wattron(win_map, COLOR_PAIR(player->color_pair));
        mvwaddch(win_map, player->y, player->x, player->symbol);
        wattroff(win_map, COLOR_PAIR(player->color_pair));
    }
}

void ui_render_stats(const Entity* player) {
    wattron(win_panel, COLOR_PAIR(2));
    box(win_panel, 0, 0);
    mvwprintw(win_panel, 1, 2, "Name: %s", player->name);
    mvwprintw(win_panel, 3, 2, "HP: %d/%d", player->resources.hp, player->resources.max_hp);
    mvwprintw(win_panel, 4, 2, "TP: %d", player->resources.tp);
    mvwprintw(win_panel, 6, 2, "Time: %ld", turn_get_current_time());
    
    if (player->is_engaged) {
        mvwprintw(win_panel, 8, 2, "[ENGAGED]");
    }
    wattroff(win_panel, COLOR_PAIR(2));
}

void ui_render_log(void) {
    wattron(win_log, COLOR_PAIR(2));
    // Render last N lines
    int y = 0;
    
    int start_idx = (log_count > LOG_HEIGHT) ? (log_count - LOG_HEIGHT) : 0;
    for (int i = start_idx; i < log_count; i++) {
        mvwprintw(win_log, y++, 1, "%s", log_history[i]);
    }
    wattroff(win_log, COLOR_PAIR(2));
}

void ui_render_input_line(const char* current_input) {
    wattron(win_input, COLOR_PAIR(2));
    mvwprintw(win_input, 0, 0, "> %s", current_input);
    wattroff(win_input, COLOR_PAIR(2));
}

void ui_refresh(void) {
    wrefresh(win_map);
    wrefresh(win_panel);
    wrefresh(win_log);
    wrefresh(win_input);
}

void ui_log(const char* fmt, ...) {
    char buf[64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (log_count < MAX_LOG_LINES) {
        strcpy(log_history[log_count++], buf);
    } else {
        // Shift up
        for (int i=1; i < MAX_LOG_LINES; i++) {
            strcpy(log_history[i-1], log_history[i]);
        }
        strcpy(log_history[MAX_LOG_LINES-1], buf);
    }
}

int ui_get_input(char* input_buffer, int max_len) {
    (void) input_buffer;
    (void) max_len;
    // For ncurses, we can use wgetch(win_input)
    // But we want to support typing a command.
    
    // Simple mode: Single key
    // If user presses '/', we might enter "String Mode"
    // For now, let's just return the keycode
    
    // We need to set echo if we want to show typing?
    // Or we handle it manually.
    
    curs_set(1); // Show cursor for input
    int ch = wgetch(win_input); // Blocking
    curs_set(0);
    
    return ch;
}

void ui_get_string(char* buffer, int max_len) {
    echo();
    curs_set(1);
    
    // Move to input window
    wmove(win_input, 0, 2); // after "> "
    wgetnstr(win_input, buffer, max_len);
    
    noecho();
    curs_set(0);
}
