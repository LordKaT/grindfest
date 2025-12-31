#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "ui.h"
#include "turn.h"
#include "map.h"
#include "entity.h"
#include "input.h"

Game g_game;

void game_init(void) {
    memset(&g_game, 0, sizeof(Game));
    g_game.running = true;
    g_game.current_state = STATE_START_MENU;
    
    // Init modules
    ui_init();
    turn_init();
    
    // Stub player init
    g_game.player.id = 0;
    strcpy(g_game.player.name, "Adventurer");
    g_game.player.symbol = '@';
    g_game.player.color_pair = 1;
    g_game.player.resources.hp = 100;
    g_game.player.resources.max_hp = 100;
    g_game.player.resources.tp = 0;
    g_game.player.resources.max_tp = 3000;
    g_game.player.is_active = true;
    
    // Add Dummy Monster
    g_game.entity_count = 1; // 0 is player
    Entity* mob = &g_game.entities[0]; // Wait, logic says entities[0] is player?
    // game_get_entity(0) returns &g_game.player.
    // g_game.entities array is for OTHER entities.
    // Let's stick to that convention.
    
    mob->id = 1;
    strcpy(mob->name, "Goblin");
    mob->symbol = 'g';
    mob->color_pair = 3; // Red
    mob->x = 10;
    mob->y = 10;
    mob->resources.hp = 50;
    mob->resources.max_hp = 50;
    mob->is_active = true;
    g_game.entity_count = 1;

    // Generate Dungeon
    map_generate_dungeon(&g_game.current_map);

    // Place Player on random floor
    int placed = 0;
    while(!placed) {
        int rx = rand() % MAP_WIDTH;
        int ry = rand() % MAP_HEIGHT;
        if (map_is_walkable(&g_game.current_map, rx, ry)) {
            g_game.player.x = rx;
            g_game.player.y = ry;
            placed = 1;
        }
    }
}

void game_cleanup(void) {
    ui_cleanup();
}

Entity* game_get_entity(EntityID id) {
    if (id == 0) return &g_game.player;
    // Simple linear search for scaffold
    for (int i=0; i < g_game.entity_count; i++) {
        if (g_game.entities[i].id == id) return &g_game.entities[i];
    }
    return NULL;
}

// --- States ---

static void update_start_menu(void) {
    ui_clear();
    ui_log("Welcome to Grindfest. Press Any Key.");
    ui_render_log();
    ui_refresh();
    
    char dummy[10];
    ui_get_input(dummy, 10); // blocking wait
    
    g_game.current_state = STATE_CHAR_CREATOR;
}

static void update_char_creator(void) {
    ui_clear();
    ui_log("Character Creation (Stub). Press Any Key.");
    ui_render_log();
    ui_refresh();

    char dummy[10];
    ui_get_input(dummy, 10);
    
    // Setup initial turn
    turn_add_event(0, g_game.player.id, EVENT_MOVE);
    
    g_game.current_state = STATE_DUNGEON_LOOP;
}


static void update_dungeon(void) {
    if (turn_queue_is_empty()) {
        // Should not happen if strictly circular, but safety
        turn_add_event(turn_get_current_time() + 100, g_game.player.id, EVENT_MOVE);
    }

    GameEvent evt = turn_pop_event();
    Entity* e = game_get_entity(evt.entity_id);
    if (!e) return; // Entity might have died/vanished

    if (evt.type == EVENT_ATTACK_READY) {
        // Process Auto Attack
        if (e->is_engaged) {
            ui_log("%s auto-attacks!", e->name);
            // Schedule next attack
            turn_add_event(evt.time + e->weapon_delay, e->id, EVENT_ATTACK_READY);

        }
    } else if (evt.type == EVENT_MOVE) {
        if (e->id == g_game.player.id) {
            // Player Turn: Block for input
            // But we must permit the render loop to draw while waiting? 
            // Ncurses `getch` blocks. 
            // If we block here, background animations (if any) stop. 
            // But for a roguelike, freezing the world until player acts is standard.
            
            // PROBLEM: "Movement and Attacking are separate event chains"
            // If Player is Auto-Attacking, that event might be in the future.
            // If we block NOW for movement, time stops. 
            // That is correct for a roguelike. Even if "auto attacking", the attack happens at specific ticks.
            // Those ticks won't process until we finish the current turn (Time += cost).
            
            // Loop until valid action taken
            bool turn_taken = false;
            while (!turn_taken) {
                // Render
                ui_clear();
                ui_render_map(&g_game.current_map, &g_game.player, g_game.entities, g_game.entity_count);
                ui_render_stats(&g_game.player);
                ui_render_log();
                ui_render_input_line(""); // Clear input line
                ui_refresh();
                
                char buf[256] = {0};
                int key = ui_get_input(buf, 256);
                
                InputResult res = input_handle_key(key);
                
                int dx = 0, dy = 0;
                if (res.type == INPUT_ACTION_QUIT) {
                    g_game.running = false;
                    turn_taken = true;
                }
                else if (res.type == INPUT_ACTION_MOVE_UP) dy = -1;
                else if (res.type == INPUT_ACTION_MOVE_DOWN) dy = 1;
                else if (res.type == INPUT_ACTION_MOVE_LEFT) dx = -1;
                else if (res.type == INPUT_ACTION_MOVE_RIGHT) dx = 1;
                else if (res.type == INPUT_ACTION_MOVE_UP_LEFT) { dx = -1; dy = -1; }
                else if (res.type == INPUT_ACTION_MOVE_UP_RIGHT) { dx = 1; dy = -1; }
                else if (res.type == INPUT_ACTION_MOVE_DOWN_LEFT) { dx = -1; dy = 1; }
                else if (res.type == INPUT_ACTION_MOVE_DOWN_RIGHT) { dx = 1; dy = 1; }
                else if (res.type == INPUT_ACTION_COMMAND) {
                    // Enter command mode
                    ui_render_input_line("/");
                    ui_refresh();
                    
                    char cmd_buf[128];
                    ui_get_string(cmd_buf, 128);
                    
                    // Prepend / to match expected format if user typed "attack" vs "/attack"?
                    // ui_get_string captures what they typed.
                    // prompt says "/attack".
                    // Let's assume they type "attack" after the prompt /
                    // Construct full string
                    char full_cmd[130];
                    sprintf(full_cmd, "/%s", cmd_buf);
                    
                    input_parse_command(full_cmd, &g_game.player, NULL);
                    
                    // Commands take a turn? Maybe.
                    turn_taken = true; 
                    turn_add_event(evt.time + 50, e->id, EVENT_MOVE); // fast action
                }

                if (dx != 0 || dy != 0) {
                     int nx = e->x + dx;
                     int ny = e->y + dy;
                     
                     if (map_is_walkable(&g_game.current_map, nx, ny)) {
                         e->x = nx;
                         e->y = ny;
                         turn_taken = true;
                         // Movement cost
                         // Use Entity stats later
                         turn_add_event(evt.time + 100, e->id, EVENT_MOVE);
                     }
                }
            }
            
        } else {
            // AI Move
            // Stub AI
            turn_add_event(evt.time + 110, e->id, EVENT_MOVE);
        }
    }
}

void game_run(void) {
    while (g_game.running) {
        switch (g_game.current_state) {
            case STATE_START_MENU: update_start_menu(); break;
            case STATE_CHAR_CREATOR: update_char_creator(); break;
            case STATE_DUNGEON_LOOP: update_dungeon(); break;
            case STATE_GAME_OVER: g_game.running = false; break;
        }
    }
}
