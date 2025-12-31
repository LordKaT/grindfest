#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "ai.h"
#include "turn.h"
#include "combat.h" // For engage if needed, though we might just set state

// Helpers
static bool ai_can_see_target(Map* map, Entity* observer, Entity* target);
static void ai_worm_update(Entity* e, Map* map, Game* game);

void ai_take_turn(Entity* e, Map* map, Game* game) {
    if (!e->is_active) return;

    // Dispatch based on Race/Job
    if (e->race == RACE_WORM) {
        ai_worm_update(e, map, game);
    } else {
        // Fallback / Generic AI
        // For now, just schedule next turn to prevent hanging
        turn_add_event(turn_get_current_time() + e->move_speed, e->id, EVENT_MOVE);
    }
}

// ----------------------------------------------------------------------------
// Sensory Helpers
// ----------------------------------------------------------------------------

// Bresenham's Line Algorithm for Line of Sight
__attribute__((unused)) static bool ai_can_see_target(Map* map, Entity* observer, Entity* target) {
    int x0 = observer->x;
    int y0 = observer->y;
    int x1 = target->x;
    int y1 = target->y;
    
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    
    while (true) {
        if (x0 == x1 && y0 == y1) return true; // Reached target
        
        // Check obstruction (don't check start tile)
        if (x0 != observer->x || y0 != observer->y) { // Start tile is observer
             // For LoS, walls block.
             if (map->tiles[x0][y0].type == TILE_WALL) return false;
        }

        if (2 * err >= dy) { err += dy; x0 += sx; }
        if (2 * err <= dx) { err += dx; y0 += sy; }
    }
}

// ----------------------------------------------------------------------------
// Specific AI Implementations
// ----------------------------------------------------------------------------

static void ai_worm_update(Entity* e, Map* map, Game* game) {
    (void)game; // Unused for now
    long current_time = turn_get_current_time();
    int ticks_to_next = 100; // Default fallback cost

    // State Machine
    switch (e->ai_state) {
        case AI_IDLE: {
            // Wait random duration (15-45 turns)
            int turns = 15 + (rand() % 31);
            ticks_to_next = turns * 100;
            
            // Transition -> Burrowing
            e->ai_state = AI_BURROWING;
            // ui_log("The ground trembles..."); // Debug/Flavor?
            break;
        }
        
        case AI_BURROWING: {
            // Dig into ground
            e->is_burrowed = true;
            map_set_occupied(map, e->x, e->y, false); // Free old tile
            
            // Pick destination
            int attempts = 0;
            int dest_x = e->x, dest_y = e->y;
            // bool found = false;
            
            while (attempts < 20) {
                int rx = (rand() % 17) - 8; // -8 to 8
                int ry = (rand() % 17) - 8;
                int tx = e->x + rx;
                int ty = e->y + ry;
                
                if (map_is_walkable(map, tx, ty) && !map_is_occupied(map, tx, ty)) {
                    dest_x = tx;
                    dest_y = ty;
                    // found = true;
                    break;
                }
                attempts++;
            }
            
            e->burrow_dest_x = dest_x;
            e->burrow_dest_y = dest_y;
            
            // Calculate travel time
            int dist = abs(dest_x - e->x) + abs(dest_y - e->y);
            ticks_to_next = dist * e->move_speed;
            
            // Transition -> Travel
            e->ai_state = AI_WORM_TRAVEL;
            break;
        }
        
        case AI_WORM_TRAVEL: {
            // Resurface
            e->x = e->burrow_dest_x;
            e->y = e->burrow_dest_y;
            e->is_burrowed = false;
            map_set_occupied(map, e->x, e->y, true); // Occupy new tile
            
            // Transition -> Idle
            e->ai_state = AI_IDLE;
            ticks_to_next = 10; // Short pause after surfacing
            break;
        }

        case AI_ENGAGED: {
            // Aggressive tracking logic (Smell)
            // Pick neighbor with highest smell
            // int best_smell = -1;
            int target_x = e->x;
            int target_y = e->y;
            
            int dirs[4][2] = {{0,-1}, {0,1}, {-1,0}, {1,0}}; // N, S, W, E
            
            // Randomize start index to avoid bias? 
            // Or iterate and collect candidates.
            int candidates[4] = {-1, -1, -1, -1};
            int candidate_count = 0;
            int max_val = -1;

            for (int i=0; i<4; i++) {
                int nx = e->x + dirs[i][0];
                int ny = e->y + dirs[i][1];
                
                if (map_is_walkable(map, nx, ny) && !map_is_occupied(map, nx, ny)) {
                    int val = map->smell[nx][ny];
                    if (val > max_val) {
                        max_val = val;
                        candidate_count = 0;
                        candidates[candidate_count++] = i;
                    } else if (val == max_val) {
                        candidates[candidate_count++] = i;
                    }
                }
            }
            
            if (max_val > 0 && candidate_count > 0) {
                 // Pick random winner
                 int win_idx = candidates[rand() % candidate_count];
                 target_x = e->x + dirs[win_idx][0];
                 target_y = e->y + dirs[win_idx][1];
                 
                 // Move
                 map_set_occupied(map, e->x, e->y, false);
                 e->x = target_x;
                 e->y = target_y;
                 map_set_occupied(map, e->x, e->y, true);
                 
                 ticks_to_next = e->move_speed;
            } else {
                // Lost scent or blocked
                // Give up
                e->ai_state = AI_IDLE;
                ticks_to_next = 100;
            }
            break;
        }
        
        default:
            e->ai_state = AI_IDLE; 
            break;
    }

    turn_add_event(current_time + ticks_to_next, e->id, EVENT_MOVE);
}
