#include <stdlib.h>
#include <stdio.h> // for logging/debug if needed
#include <math.h>
#include "map.h"


void map_generate_dungeon(Map* map) {
    // 1. Initialize all to WALL
    for(int x=0; x<MAP_WIDTH; x++) {
        for(int y=0; y<MAP_HEIGHT; y++) {
            map->tiles[x][y].type = TILE_WALL;
            map->tiles[x][y].visible = false;
            map->tiles[x][y].explored = false;
            map->tiles[x][y].occupied = false;
        }
    }

    // 2. Drunken Walk
    int total_cells = (MAP_WIDTH - 2) * (MAP_HEIGHT - 2);
    int target_floors = total_cells * 40 / 100; // 40% coverage
    int floors_count = 0;

    int cx = MAP_WIDTH / 2;
    int cy = MAP_HEIGHT / 2;

    int max_iters = target_floors * 10; // Safety break
    int iter = 0;

    while (floors_count < target_floors && iter < max_iters) {
        if (map->tiles[cx][cy].type == TILE_WALL) {
            map->tiles[cx][cy].type = TILE_FLOOR;
            floors_count++;
        }

        // Random direction
        int dir = rand() % 4;
        int dx = 0, dy = 0;
        switch(dir) {
            case 0: dy = -1; break; // Up
            case 1: dy = 1;  break; // Down
            case 2: dx = -1; break; // Left
            case 3: dx = 1;  break; // Right
        }

        int nx = cx + dx;
        int ny = cy + dy;

        // Clamp to border (keep 1-tile border)
        if (nx < 1) nx = 1;
        if (nx > MAP_WIDTH - 2) nx = MAP_WIDTH - 2;
        if (ny < 1) ny = 1;
        if (ny > MAP_HEIGHT - 2) ny = MAP_HEIGHT - 2;

        cx = nx;
        cy = ny;
        iter++;
    }

    // 3. Post-Generation Cleanup (Remove Isolated Walls)
    bool changes = true;
    while(changes) {
        changes = false;
        for(int x=1; x<MAP_WIDTH-1; x++) {
            for(int y=1; y<MAP_HEIGHT-1; y++) {
                if(map->tiles[x][y].type == TILE_WALL) {
                    int neighbor_walls = 0;
                    if(map->tiles[x][y-1].type == TILE_WALL) neighbor_walls++;
                    if(map->tiles[x][y+1].type == TILE_WALL) neighbor_walls++;
                    if(map->tiles[x-1][y].type == TILE_WALL) neighbor_walls++;
                    if(map->tiles[x+1][y].type == TILE_WALL) neighbor_walls++;

                    if(neighbor_walls == 0) {
                        map->tiles[x][y].type = TILE_FLOOR; // Flip isolated wall to floor
                        changes = true;
                    }
                }
            }
        }
    }
    
    // 4. Connectivity Check (Flood Fill) - Implicitly handled by Drunken Walk
}

// Field of View (Recursive Shadowcasting)

// Raycasting fallback (Simple, robust)
void map_compute_fov(Map* map, int px, int py, int radius) {
    // 1. Reset visibility
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            map->tiles[x][y].visible = false;
        }
    }
    
    // 2. Mark player tile visible
    if (px >= 0 && px < MAP_WIDTH && py >= 0 && py < MAP_HEIGHT) {
        map->tiles[px][py].visible = true;
        map->tiles[px][py].explored = true;
    }

    // 3. Cast rays to perimeter of square 2*radius
    for (int i = -radius; i <= radius; i++) {
        // Top/Bottom
        int targets[2][2] = { {px + i, py - radius}, {px + i, py + radius} };
        // Left/Right
        int targets2[2][2] = { {px - radius, py + i}, {px + radius, py + i} };
        
        for(int k=0; k<2; k++) { // Top/Bot
            float dx = targets[k][0] - px;
            float dy = targets[k][1] - py;
            float dist = sqrtf(dx*dx + dy*dy);
            float stepX = dx / dist;
            float stepY = dy / dist;
            
            float curX = px + 0.5f;
            float curY = py + 0.5f;
            
            for(int step=0; step<=radius; step++) {
                int tx = (int)curX;
                int ty = (int)curY;
                
                if (tx < 0 || tx >= MAP_WIDTH || ty < 0 || ty >= MAP_HEIGHT) break;
                
                // Distance check
                if ((tx-px)*(tx-px) + (ty-py)*(ty-py) > radius*radius) break;

                map->tiles[tx][ty].visible = true;
                map->tiles[tx][ty].explored = true;
                
                if (map->tiles[tx][ty].type == TILE_WALL) {
                    break; // Block sight
                }
                
                curX += stepX;
                curY += stepY;
            }
        }
         for(int k=0; k<2; k++) { // Left/Right
            float dx = targets2[k][0] - px;
            float dy = targets2[k][1] - py;
            float dist = sqrtf(dx*dx + dy*dy);
            float stepX = dx / dist;
            float stepY = dy / dist;
            
            float curX = px + 0.5f;
            float curY = py + 0.5f;
            
            for(int step=0; step<=radius; step++) {
                int tx = (int)curX;
                int ty = (int)curY;
                
                if (tx < 0 || tx >= MAP_WIDTH || ty < 0 || ty >= MAP_HEIGHT) break;
                
                // Distance check
                if ((tx-px)*(tx-px) + (ty-py)*(ty-py) > radius*radius) break;

                map->tiles[tx][ty].visible = true;
                map->tiles[tx][ty].explored = true;
                
                if (map->tiles[tx][ty].type == TILE_WALL) {
                    break; // Block sight
                }
                
                curX += stepX;
                curY += stepY;
            }
        }
    }
}

// Sensory Systems (Smell / Sound)

void map_update_smell(Map* map, int px, int py) {
    // 1. Global Decay
    for(int x=0; x<MAP_WIDTH; x++) {
        for(int y=0; y<MAP_HEIGHT; y++) {
            if (map->smell[x][y] > 30) 
                 map->smell[x][y] -= 30;
            else 
                 map->smell[x][y] = 0;
        }
    }

    // 2. Source (Player)
    if (px >= 0 && px < MAP_WIDTH && py >= 0 && py < MAP_HEIGHT) {
        map->smell[px][py] = 255;
    }

    // 3. Diffusion Pass (using temp buffer to avoid directional bias)
    uint8_t next_smell[MAP_WIDTH][MAP_HEIGHT];
    // Copy current state
    for(int x=0; x<MAP_WIDTH; x++) {
        for(int y=0; y<MAP_HEIGHT; y++) {
            next_smell[x][y] = map->smell[x][y];
        }
    }

    uint8_t drop_off = 80; // Diffusion loss

    for(int x=1; x<MAP_WIDTH-1; x++) {
        for(int y=1; y<MAP_HEIGHT-1; y++) {
            if (map->tiles[x][y].type == TILE_WALL) continue; // Walls don't diffuse

            // Check neighbors
            uint8_t max_n = 0;
            // N
            if (map->smell[x][y-1] > max_n) max_n = map->smell[x][y-1];
            // S
            if (map->smell[x][y+1] > max_n) max_n = map->smell[x][y+1];
            // E
            if (map->smell[x+1][y] > max_n) max_n = map->smell[x+1][y];
            // W
            if (map->smell[x-1][y] > max_n) max_n = map->smell[x-1][y];

            // Absorb
            if (max_n > drop_off) {
                uint8_t diffused = max_n - drop_off;
                if (diffused > next_smell[x][y]) {
                    next_smell[x][y] = diffused;
                }
            }
        }
    }

    // Apply back
    for(int x=0; x<MAP_WIDTH; x++) {
        for(int y=0; y<MAP_HEIGHT; y++) {
            map->smell[x][y] = next_smell[x][y];
        }
    }
}

// Simple BFS Queue for Sound
typedef struct {
    int x, y;
    int dist;
    int walls;
} SoundNode;

void map_update_sound(Map* map, int px, int py, int radius) {
    // 1. Reset
    for(int x=0; x<MAP_WIDTH; x++) {
        for(int y=0; y<MAP_HEIGHT; y++) {
            map->sound[x][y] = SOUND_NONE;
        }
    }

    if (px < 0 || px >= MAP_WIDTH || py < 0 || py >= MAP_HEIGHT) return;

    // BFS
    // Max queue size = map size (overkill but safe stack usage)
    // Actually stack might be small? 54*16 = 864 structs. 
    // Struct is ~16 bytes. 13KB. Safe for stack.
    SoundNode queue[MAP_WIDTH * MAP_HEIGHT];
    int head = 0;
    int tail = 0;
    bool visited[MAP_WIDTH][MAP_HEIGHT] = {false};

    queue[tail++] = (SoundNode){px, py, 0, 0};
    visited[px][py] = true;
    map->sound[px][py] = SOUND_CLEAR;

    int dirs[4][2] = { {0,-1}, {0,1}, {-1,0}, {1,0} };

    while (head < tail) {
        SoundNode curr = queue[head++];
        
        if (curr.dist >= radius) continue;

        for (int i=0; i<4; i++) {
            int nx = curr.x + dirs[i][0];
            int ny = curr.y + dirs[i][1];

            if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT) continue;
            if (visited[nx][ny]) continue;

            // Wall check
            int new_walls = curr.walls;
            if (map->tiles[nx][ny].type == TILE_WALL) {
                new_walls++;
            }

            // Propagate constraints
            // Can pass 1 wall (becomes muffled). 
            // If already passed 1 wall and hits another, stops
            if (new_walls >= 2) continue; 

            visited[nx][ny] = true;
            
            // Set State
            if (new_walls == 0) map->sound[nx][ny] = SOUND_CLEAR;
            else map->sound[nx][ny] = SOUND_MUFFLED;

            queue[tail++] = (SoundNode){nx, ny, curr.dist + 1, new_walls};
        }
    }
}

bool map_is_smelly(const Map* map, int x, int y) {
     if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) return false;
     return map->smell[x][y] > 0;
}

SoundState map_sound_at(const Map* map, int x, int y) {
     if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) return SOUND_NONE;
     return map->sound[x][y];
}

bool map_is_walkable(Map* map, int x, int y) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) return false;
    return map->tiles[x][y].type == TILE_FLOOR;
}

// ----------------------------------------------------------------------------
// Occupancy
// ----------------------------------------------------------------------------

void map_set_occupied(Map* map, int x, int y, bool occupied) {
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return;
    map->tiles[x][y].occupied = occupied;
}

bool map_is_occupied(Map* map, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return true; // Treat OOB as occupied
    return map->tiles[x][y].occupied;
}
