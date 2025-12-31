#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "ui.h"
#include "turn.h"
#include "map.h"
#include "entity.h"
#include "input.h"
#include "ai.h"

Game g_game;

void game_init(void) {
    memset(&g_game, 0, sizeof(Game));
    g_game.running = true;
    g_game.current_state = STATE_START_MENU;
    g_game.render_mode = RENDER_MODE_NORMAL;
    
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
    g_game.player.resources.max_tp = 3000;
    g_game.player.is_active = true;
    
    // Persistence Init
    g_game.player.claimed_by = -1;
    g_game.player.main_job = JOB_WARRIOR;
    g_game.player.job_levels[JOB_WARRIOR] = 1;
    g_game.player.current_level = 1;
    g_game.player.job_exp[JOB_WARRIOR] = 0;
    
    // Stats (Stub values)
    g_game.player.base_stats.str = 10;
    g_game.player.base_stats.dex = 10;
    g_game.player.base_stats.vit = 10;
    g_game.player.current_stats = g_game.player.base_stats;
    
    // Generate Dungeon first to ensure floors exist
    map_generate_dungeon(&g_game.current_map);

    // Place Player on random floor
    int placed = 0;
    while(!placed) {
        int rx = rand() % MAP_WIDTH;
        int ry = rand() % MAP_HEIGHT;
        if (map_is_walkable(&g_game.current_map, rx, ry) && !map_is_occupied(&g_game.current_map, rx, ry)) {
            g_game.player.x = rx;
            g_game.player.y = ry;
            map_set_occupied(&g_game.current_map, rx, ry, true);
            placed = 1;
        }
    }
    
    // Spawn Worm
    g_game.entity_count = 1; // 0 is player
    int attempts = 0;
    while (attempts < 50) {
        int rx = rand() % MAP_WIDTH;
        int ry = rand() % MAP_HEIGHT;
        if (map_is_walkable(&g_game.current_map, rx, ry) && !map_is_occupied(&g_game.current_map, rx, ry)) {
              Entity* e = &g_game.entities[0]; // First enemy
              e->id = 1;
              e->type = ENTITY_ENEMY;
              e->race = RACE_WORM;
              // e->job = JOB_WORM; // JobType? Not critical if not used yet
              strcpy(e->name, "Tunnel Worm");
              e->symbol = 'w';
              e->color_pair = 3; // Red
              e->x = rx;
              e->y = ry;
              e->resources.hp = 100;
              e->resources.max_hp = 100;
              e->is_active = true;
              e->move_speed = 100;
              e->is_aggressive = false; // Passive
              e->detection_flags = DETECT_SMELL;
              e->ai_state = AI_IDLE;
              e->is_burrowed = false;
              
              // Persistence
              e->claimed_by = -1;
              e->current_level = 4;
              e->job_levels[e->main_job] = 4;
              e->base_stats.str = 8;
              e->current_stats = e->base_stats;
              
              map_set_occupied(&g_game.current_map, rx, ry, true);
              
              // Schedule first move
              turn_add_event(100, e->id, EVENT_MOVE);
              
              g_game.entity_count = 1; 
              break;
        }
        attempts++;
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
    
    if (evt.type == EVENT_MOVE) {
         // Tick Status Effects whenever a move turn comes up
         entity_tick_status(e);
    }

    if (evt.type == EVENT_ATTACK_READY) {
        // Process Auto Attack
        // Note: If Player Move and Attack happen at same time, Priority ID (insertion order)
        // determines order. Scheduler executes earlier enqueued event first.
        if (e->is_engaged) {
            ui_log("%s auto-attacks!", e->name);
            // Schedule next attack
            turn_add_event(evt.time + e->weapon_delay, e->id, EVENT_ATTACK_READY);

        }
    } else if (evt.type == EVENT_MOVE) {
        if (e->id == g_game.player.id) {
            // Player Turn: Input Loop
            // The game waits indefinitely for player input here.
            // Render loop runs to keep UI fresh, but game time (simulation) is paused.
            
            // Turn Logic (turn-cost units, not real-time):
            // 1. Pop the next event by scheduled time.
            // 2. If it is the player's Move event, block for input.
            // 3. Input determines action cost and schedules the next Move event.
            // 4. Any intermediate events (like auto-attacks) scheduled between now and the
            //    next Move event will be popped and processed in order before input resumes.
            
            // Loop until valid action taken
            bool turn_taken = false;
            while (!turn_taken) {
                // Update FOV
                map_compute_fov(&g_game.current_map, g_game.player.x, g_game.player.y, FOV_RADIUS);

                // Update visuals (FOV, Render).
                // Note: Simulation state (Smell/Sound) is NOT updated here.
                // It only updates when 'turn_taken' becomes true.
                
                // Render
                ui_clear();
                ui_render_map(&g_game.current_map, &g_game.player, g_game.entities, g_game.entity_count, g_game.render_mode);
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
                else if (res.type == INPUT_ACTION_VIEW_NORMAL) g_game.render_mode = RENDER_MODE_NORMAL;
                else if (res.type == INPUT_ACTION_VIEW_SMELL) g_game.render_mode = RENDER_MODE_SMELL;
                else if (res.type == INPUT_ACTION_VIEW_SOUND) g_game.render_mode = RENDER_MODE_SOUND;
                else if (res.type == INPUT_ACTION_WAIT) {
                    ui_log("You wait.");
                    turn_taken = true;
                    // Standard wait cost (100)
                    turn_add_event(evt.time + 100, e->id, EVENT_MOVE);
                }
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
                    
                    // Commands take a turn? Maybe. Depends on the executed command.
                    // Do not increment here.
                    turn_taken = false;
                    //turn_add_event(evt.time + 50, e->id, EVENT_MOVE); // fast action
                }

                if (dx != 0 || dy != 0) {
                     int nx = e->x + dx;
                     int ny = e->y + dy;
                     
                     if (map_is_walkable(&g_game.current_map, nx, ny)) {
                         // Check occupancy
                         if (!map_is_occupied(&g_game.current_map, nx, ny)) {
                             // Move
                             map_set_occupied(&g_game.current_map, e->x, e->y, false);
                             e->x = nx;
                             e->y = ny;
                             map_set_occupied(&g_game.current_map, e->x, e->y, true);
                             
                             turn_taken = true;
                             // Smell update logic
                             map_update_smell(&g_game.current_map, g_game.player.x, g_game.player.y);
                             // Sound update logic (already handled by loop re-entry? no, instantaneous)
                             map_update_sound(&g_game.current_map, g_game.player.x, g_game.player.y, 5);
    
                             // Movement cost
                             // Use Entity stats later
                             turn_add_event(evt.time + 100, e->id, EVENT_MOVE);
                         } else {
                             // Blocked by entity?
                             // Maybe attack?
                             // For now, simple block log
                             ui_log("Blocked.");
                         }
                     }
                }
            }
            
        } else {
            // AI Move
            if (e->type == ENTITY_ENEMY) {
                ai_take_turn(e, &g_game.current_map, &g_game);
            } else {
                 turn_add_event(evt.time + 100, e->id, EVENT_MOVE);
            }
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
