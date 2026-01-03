#define _XOPEN_SOURCE_EXTENDED 1
#define _DEFAULT_SOURCE
#include <ncurses.h>
#include <locale.h>
#include <stdarg.h>
#include <stdlib.h> // for setenv

#include <string.h>
#include "ui.h"
#include "turn.h"

// Layout Definitions
// Defaults for Game Loop
// MAP 54, PANEL 26
// Defaults for Creator
// MAP 40, PANEL 40

static int layout_map_width = 54;
static int layout_panel_width = 26;
// Heights are constant for now
#define MAP_VIEW_HEIGHT 17
#define LOG_HEIGHT 6
#define INPUT_HEIGHT 1
#define PANEL_HEIGHT 24

static WINDOW *win_map;
static WINDOW *win_panel;
static WINDOW *win_log;
static WINDOW *win_input;
static WINDOW *win_menu; // Menu Window

// Simple log buffer
#define MAX_LOG_LINES 50
static char log_history[MAX_LOG_LINES][64];
//static int log_head = 0;
static int log_count = 0;
static int animation_frame = 0;

void ui_init(void) {
    // Reduce ESC delay to 25ms to prevent menu exit lag
    setenv("ESCDELAY", "25", 1);
    
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
        // Water Animation Colors
        init_pair(10, COLOR_BLUE, COLOR_BLACK);
        init_pair(11, COLOR_CYAN, COLOR_BLACK);
        init_pair(10, COLOR_BLUE, COLOR_BLACK);
        init_pair(11, COLOR_CYAN, COLOR_BLACK);
        init_pair(12, COLOR_WHITE, COLOR_BLACK);
        init_pair(14, COLOR_MAGENTA, COLOR_BLACK); // Teleport

        // Bridge/Wood
        //init_pair(13, COLOR_YELLOW, COLOR_BLACK);
        if (can_change_color() && COLORS >= 16) {
            init_color(8, 600, 300, 0);   // R, G, B
            init_pair(13, 8, COLOR_BLACK);
        } else {
            init_pair(1, COLOR_YELLOW, COLOR_BLACK);
        }
    }

    bkgd(COLOR_PAIR(2)); // Force stdscr to black
    refresh(); // Refresh stdscr

    // Create Windows

    // Initial Layout is Game Default? Or Creator?
    // We'll let game logic call ui_set_layout.
    // For init, we don't create windows yet. ui_set_layout will do it.
    // But we need to be safe if render called before layout.
    
    ui_set_layout(UI_LAYOUT_GAME); // Default
}

void ui_set_layout(UILayout layout) {
    if (layout == UI_LAYOUT_GAME) {
        layout_map_width = 54;
        layout_panel_width = 26;
    } else if (layout == UI_LAYOUT_CREATOR) {
        layout_map_width = 40;
        layout_panel_width = 40;
    }

    if (win_map) delwin(win_map);
    if (win_panel) delwin(win_panel);
    if (win_log) delwin(win_log);
    if (win_input) delwin(win_input);
    if (win_menu) { delwin(win_menu); win_menu = NULL; }

    win_map = newwin(MAP_VIEW_HEIGHT, layout_map_width, 0, 0);
    win_panel = newwin(PANEL_HEIGHT, layout_panel_width, 0, layout_map_width);
    win_log = newwin(LOG_HEIGHT, layout_map_width, MAP_VIEW_HEIGHT, 0);
    win_input = newwin(INPUT_HEIGHT, layout_map_width, MAP_VIEW_HEIGHT + LOG_HEIGHT, 0);
    
    keypad(win_input, TRUE);

    wbkgd(win_map, COLOR_PAIR(2));
    wbkgd(win_panel, COLOR_PAIR(2));
    wbkgd(win_log, COLOR_PAIR(2));
    wbkgd(win_input, COLOR_PAIR(2));
    
    refresh();

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
    if (win_menu) delwin(win_menu);
    endwin();
}

void ui_clear(void) {
    werase(win_map);
    werase(win_panel);
    werase(win_log);
    werase(win_input);
    if (win_menu) werase(win_menu);
    
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
    if (x < 0 || y < 0 || x >= map->width || y >= map->height) return false;
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

static void mvwadd_wchar(WINDOW* win, int y, int x, wchar_t wch) {
    cchar_t c;
    wchar_t w[2] = {wch, 0};
    setcchar(&c, w, A_NORMAL, 0, NULL);
    mvwadd_wch(win, y, x, &c);
}

void ui_render_map(Map* map, const Entity* player, const Entity entities[], int entity_count, RenderMode mode) {
    // Basic rendering 
    werase(win_map);

    // Draw Title Bar
    wattron(win_map, A_BOLD);
    mvwprintw(win_map, 0, 0, "[ %s ]", map->name);
    wattroff(win_map, A_BOLD);

    scrollok(win_map, FALSE);

    // Camera Calculation
    int cam_x = player->x - (layout_map_width / 2);
    int cam_y = player->y - (MAP_VIEW_HEIGHT / 2);

    // Clamping
    int max_cam_x = map->width - layout_map_width;
    int max_cam_y = map->height - MAP_VIEW_HEIGHT;

    if (max_cam_x < 0) max_cam_x = 0;
    if (max_cam_y < 0) max_cam_y = 0;

    if (cam_x < 0) cam_x = 0;
    if (cam_y < 0) cam_y = 0;
    if (cam_x > max_cam_x) cam_x = max_cam_x;
    if (cam_y > max_cam_y) cam_y = max_cam_y;

    // Draw Map (Viewport Loop)
    for (int vy = 0; vy < MAP_VIEW_HEIGHT; vy++) {
        // Map Y coordinate
        int y = cam_y + vy;
        
        // Window Y (account for title bar offset +1)
        // Wait, title bar is at 0. Map starts at 1.
        // But MAP_VIEW_HEIGHT includes the title bar row?
        // No, MAP_VIEW_HEIGHT is the total height of the window (17).
        // If we draw map rows 0..15 (16 rows) starting at win_y 1..16.
        // We should loop vy from 0 to MAP_VIEW_HEIGHT - 2 ? (Total 16 lines).
        
        // Let's stick to the previous logic:
        // win_y = vy + 1. If win_y >= MAP_VIEW_HEIGHT continue.
        // But previously 'y' was the map coordinate.
        // Now 'vy' is viewport relative map offset.
        
        int win_y = vy + 1;
        if (win_y >= MAP_VIEW_HEIGHT) continue;
        if (y >= map->height) continue;

        for (int vx = 0; vx < layout_map_width; vx++) {
            // Map X coordinate
            int x = cam_x + vx;
            if (x >= map->width) continue;

            // Render Mode Logic
            if (mode == RENDER_MODE_SMELL) {
                 if (map->tiles[x][y].type == TILE_WALL) {
                     // Draw walls normally
                 } else {
                     // Floor
                     int smell = map->smell[x][y];
                     int color = 3; // Red
                     attr_t attrs = A_NORMAL;
                     
                     if (smell == 0) {
                         wattr_set(win_map, A_NORMAL, 2, NULL);
                         mvwaddch(win_map, win_y, vx, '.');
                         continue;
                     } 
                     
                     if (smell >= 128) attrs = A_BOLD;
                     else if (smell < 64) attrs = A_DIM;
                     
                     wattr_set(win_map, attrs, color, NULL);
                     mvwadd_wch(win_map, win_y, vx, WACS_BLOCK);
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
                        mvwadd_wch(win_map, win_y, vx, WACS_BLOCK);
                        continue;
                    } else if (sound == SOUND_MUFFLED) {
                        wattr_set(win_map, A_DIM, 6, NULL); // Cyan
                        mvwadd_wch(win_map, win_y, vx, WACS_CKBOARD);
                        continue;
                    } else {
                         wattr_set(win_map, A_NORMAL, 2, NULL);
                         mvwaddch(win_map, win_y, vx, '.');
                         continue;
                    }
                }
            }

            // Normal / Fallback Rendering
            bool visible = map->tiles[x][y].visible;
            bool explored = map->tiles[x][y].explored;
            
            if (mode != RENDER_MODE_NORMAL) {
                visible = true; 
                explored = true;
            }

            if (!visible && !explored) {
                mvwaddch(win_map, win_y, vx, ' ');
                continue;
            }

            int color = 2; // Default
            attr_t attrs = A_NORMAL;
            
            if (!visible && explored) {
                attrs |= A_DIM; 
            }

            wattr_set(win_map, attrs, color, NULL); 
            
            if (map->tiles[x][y].type == TILE_FLOOR) {
                mvwaddch(win_map, win_y, vx, '.');
            } 
            else if (map->tiles[x][y].type == TILE_WATER) {
                // Animation: Cycle colors 10, 11, 12 based on frame + position
                // Phase 0..3
                int phase = (x + y + (animation_frame / 2)) % 4;
                int color_idx = 10; // Blue
                if (phase == 1 || phase == 3) color_idx = 11; // Cyan
                if (phase == 2) color_idx = 12; // White (Sparkle)
                
                wattr_set(win_map, A_NORMAL, color_idx, NULL);
                mvwaddch(win_map, win_y, vx, '~');
            }
            else if (map->tiles[x][y].type == TILE_WALL) {
                int mask = wall_mask_at(map, x, y);
                cchar_t* wglyph = get_wall_glyph(mask);
                
                if (wglyph) {
                    mvwadd_wch(win_map, win_y, vx, wglyph);
                } else {
                    mvwaddch(win_map, win_y, vx, '#'); 
                }
            } else if (map->tiles[x][y].type == TILE_BRIDGE) {
                wattr_set(win_map, A_NORMAL, 13, NULL);
                mvwaddch(win_map, win_y, vx, '=');
            } else if (map->tiles[x][y].type == TILE_ZONE) {
                wattr_set(win_map, A_NORMAL, 2, NULL);
                mvwadd_wchar(win_map, win_y, vx, 0x2591);
            } else if (map->tiles[x][y].type == TILE_TELEPORT) {
                wattr_set(win_map, A_NORMAL, 14, NULL);
                mvwadd_wchar(win_map, win_y, vx, 0x2591);
            } else {
                mvwaddch(win_map, win_y, vx, ' '); 
            }
            wattr_set(win_map, A_NORMAL, 0, NULL);
        }
    }
    
    // 2. Render Objects / NPCs / Enemies
    for (int i = 0; i < entity_count; i++) {
        if (!entities[i].is_active) continue;
        if (entities[i].id == player->id) continue;
        if (entities[i].is_burrowed) continue;
        
        // Cull
        int screen_x = entities[i].x - cam_x;
        int screen_y = entities[i].y - cam_y;
        
        if (screen_x < 0 || screen_x >= layout_map_width) continue;
        if (screen_y < 0) continue; // Check later against viewport height

        if (!map->tiles[entities[i].x][entities[i].y].visible) continue;
        
        int win_y = screen_y + 1;
        if (win_y >= MAP_VIEW_HEIGHT) continue;

        wattron(win_map, COLOR_PAIR(entities[i].color_pair));
        mvwaddch(win_map, win_y, screen_x, entities[i].symbol);
        wattroff(win_map, COLOR_PAIR(entities[i].color_pair));
    }
    
    // 3. Render Player
    if (player->is_active) {
        int screen_x = player->x - cam_x;
        int screen_y = player->y - cam_y;
        
        // Should be center typically, but clamped at edges
        
        int win_y = screen_y + 1;
        if (win_y < MAP_VIEW_HEIGHT && screen_x >= 0 && screen_x < layout_map_width) {
            wattron(win_map, COLOR_PAIR(player->color_pair));
            mvwaddch(win_map, win_y, screen_x, player->symbol);
            wattroff(win_map, COLOR_PAIR(player->color_pair));
        }
    }
}

// ----------------------------------------------------------------------------
// Creator Wizard System
// ----------------------------------------------------------------------------

void ui_render_creator_menu(const char* title, const char** items, int count, int selection, const char* description) {
    // 1. Render List in Map Window (Left)
    werase(win_map);
    box(win_map, 0, 0);
    mvwprintw(win_map, 0, 2, "[ %s ]", title);
    
    int y = 2;
    for (int i = 0; i < count; i++) {
        if (i == selection) {
            wattron(win_map, A_REVERSE | A_BOLD);
        }
        mvwprintw(win_map, y++, 2, " %s ", items[i]);
        if (i == selection) {
            wattroff(win_map, A_REVERSE | A_BOLD);
        }
    }
    wrefresh(win_map);

    // 2. Render Description in Panel Window (Right)
    werase(win_panel);
    box(win_panel, 0, 0);
    mvwprintw(win_panel, 1, 2, "Details:");
    
    // Word wrap description manually
    int desc_y = 3;
    int desc_x = 2;
    int max_width = layout_panel_width - 4;
    
    // Simple word wrap
    int len = strlen(description);
    int cursor = 0;
    while (cursor < len) {
        int space_left = max_width; 
        int chunk_len = 0;
        
        // Find how many words fit in space_left
        while (cursor + chunk_len < len) {
            // Check next word length
            int next_space = chunk_len;
            while (cursor + next_space < len && description[cursor + next_space] != ' ') {
                next_space++;
            }
            // Include reference to space
            if (cursor + next_space < len) next_space++; 
            
            if (next_space > space_left) break; // Word doesn't fit
            chunk_len = next_space;
        }
        
        // If single word is longer than line (unlikely with this width), just print it?
        if (chunk_len == 0 && space_left > 0) {
             // Force break word
             chunk_len = space_left;
        }

        mvwprintw(win_panel, desc_y++, desc_x, "%.*s", chunk_len, description + cursor);
        cursor += chunk_len;
    }
    
    wrefresh(win_panel);
    
    // 3. Clear others
    werase(win_log);
    werase(win_input);
    wrefresh(win_log);
    wrefresh(win_input);
}

// ----------------------------------------------------------------------------
// Menu System
// ----------------------------------------------------------------------------

void ui_open_menu(void) {
    if (win_menu) return; // Already open
    
    int w = 40;
    int h = 18;
    int x = (80 - w) / 2; // Center on screen (assuming 80 wide term)
    int y = (24 - h) / 2;
    
    win_menu = newwin(h, w, y, x);
    wbkgd(win_menu, COLOR_PAIR(4)); // Cyan border style
    keypad(win_menu, TRUE);
}

void ui_close_menu(void) {
    if (win_menu) {
        delwin(win_menu);
        win_menu = NULL;
    }
    // Force full repaint of underlying map next frame
    touchwin(win_map); 
}

void ui_render_menu(const Entity* player) {
    if (!win_menu) return;
    
    // "Frozen" Background:
    // We do NOT erase win_map. We just refresh it to ensure it stays visible under the menu.
    // However, ncurses might overwrite if we don't touch.
    // If we simply wrefresh(win_map) then wrefresh(win_menu), menu pops on top.
    
    // Box
    box(win_menu, 0, 0);
    
    mvwprintw(win_menu, 0, 2, "[ Status ]");
    
    // Content
    int y = 2;
    mvwprintw(win_menu, y++, 2, "Name: %s", player->name);
    y++;
    mvwprintw(win_menu, y++, 2, "Job:  %s Lv.%d", entity_get_job_name(player->main_job), player->current_level);
    mvwprintw(win_menu, y++, 2, "Race: %s", entity_get_race_name(player->race));
    y++;
    mvwprintw(win_menu, y++, 2, "HP:   %d / %d", player->resources.hp, player->resources.max_hp);
    mvwprintw(win_menu, y++, 2, "MP:   %d / %d", player->resources.mp, player->resources.max_mp);
    mvwprintw(win_menu, y++, 2, "TP:   %d", player->resources.tp);
    y++;
    mvwprintw(win_menu, y++, 2, "STR:  %d", player->current_stats.str);
    mvwprintw(win_menu, y++, 2, "DEX:  %d", player->current_stats.dex);
    mvwprintw(win_menu, y++, 2, "VIT:  %d", player->current_stats.vit);
    y++;
    mvwprintw(win_menu, y++, 2, "ATK:  %d", entity_get_derived_attack(player));
    mvwprintw(win_menu, y++, 2, "DEF:  %d", entity_get_derived_defense(player));
    
    y++;
    mvwprintw(win_menu, y++, 2, "EXP:  %d / %d", player->job_exp[player->main_job], entity_get_tnl(player->current_level));

    mvwprintw(win_menu, 16, 2, "[ESC] Close");
}

void ui_render_stats(const Entity* player) {
    wattron(win_panel, COLOR_PAIR(2));
    box(win_panel, 0, 0);
    mvwprintw(win_panel, 1, 2, "Name: %s", player->name);
    mvwprintw(win_panel, 3, 2, "HP: %d/%d", player->resources.hp, player->resources.max_hp);
    mvwprintw(win_panel, 4, 2, "TP: %d", player->resources.tp);
    mvwprintw(win_panel, 6, 2, "Time: %ld", turn_get_current_time());
    mvwprintw(win_panel, 7, 2, "Pos:  %d, %d", player->x, player->y);
    
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
    if (win_menu) wrefresh(win_menu);
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

void ui_get_string(const char* prompt, char* buffer, int max_len) {
    wtimeout(win_input, -1); // Force blocking for string input
    echo();
    curs_set(1);
    
    // Clear input line first
    werase(win_input);
    
    // Print prompt if provided
    if (prompt) {
        mvwprintw(win_input, 0, 0, "%s ", prompt);
        // The cursor stays at end of print
    } else {
        wmove(win_input, 0, 0); 
    }
    wrefresh(win_input); // Ensure prompt is visible
    
    wgetnstr(win_input, buffer, max_len);
    noecho();
    curs_set(0);
}

int ui_get_input(char* input_buffer, int max_len, int timeout_ms) {
    (void) input_buffer;
    (void) max_len;
    
    wtimeout(win_input, timeout_ms); // Set timeout
    curs_set(1); 
    int ch = wgetch(win_input);
    curs_set(0);
    
    if (ch == ERR) {
        return ERR;
    }
    
    return ch;
}

void ui_tick_animation(void) {
    animation_frame++;
}
