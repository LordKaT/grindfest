#include <stdlib.h>
#include <stdio.h> // for logging/debug if needed
#include <math.h>
#include <string.h>
#include "map.h"


void map_generate_dungeon(Map* map) {
    // Set Name
    strcpy(map->name, "Procedural Dungeon");
    // Legacy / Default Size
    map->width = 54;
    map->height = 16;
    
    // 1. Initialize all to WALL
    for(int x=0; x<map->width; x++) {
        for(int y=0; y<map->height; y++) {
            map->tiles[x][y].type = TILE_WALL;
            map->tiles[x][y].visible = false;
            map->tiles[x][y].explored = false;
            map->tiles[x][y].occupied = false;
            map->tiles[x][y].occupied = false;
        }
    }

    // 2. Drunken Walk
    int total_cells = (map->width - 2) * (map->height - 2);
    int target_floors = total_cells * 40 / 100; // 40% coverage
    int floors_count = 0;

    int cx = map->width / 2;
    int cy = map->height / 2;

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
        if (nx > map->width - 2) nx = map->width - 2;
        if (ny < 1) ny = 1;
        if (ny > map->height - 2) ny = map->height - 2;

        cx = nx;
        cy = ny;
        iter++;
    }

    // 3. Post-Generation Cleanup (Remove Isolated Walls)
    bool changes = true;
    while(changes) {
        changes = false;
        for(int x=1; x<map->width-1; x++) {
            for(int y=1; y<map->height-1; y++) {
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

void main_cleanup(void); // Forward declaration to allow abort logic? Better to just exit(1) for fatal error

// Static Map Loader
void map_load_static(Map* map, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        // Fallback or Fatal?
        // ui_log("Error loading map: %s", filename); // UI might not be ready or relevant
        fprintf(stderr, "FATAL: Could not open map file: %s\n", filename);
        exit(1);
    }
    
    char line[256];
    bool in_terrain = false;
    int y = 0;
    
    // Clear map first
    map->width = 54; // Default if not found
    map->height = 16;
    
    for(int i=0; i<MAX_MAP_WIDTH; i++) {
        for(int j=0; j<MAX_MAP_HEIGHT; j++) {
            map->tiles[i][j].type = TILE_EMPTY; // or WALL
            map->tiles[i][j].visible = false;
            map->tiles[i][j].explored = false;
            map->tiles[i][j].occupied = false;
            map->smell[i][j] = 0;
            map->sound[i][j] = SOUND_NONE;
        }
    }
    
    // Default to City for static maps
    // map->zone_type = ZONE_CITY;
    map->exit_count = 0;
    strcpy(map->name, "Unknown Area");

    while (fgets(line, sizeof(line), f)) {
        // Strip newline
        line[strcspn(line, "\r\n")] = 0;
        
        if (strlen(line) == 0) continue;
        if (line[0] == '%') continue; // Comment
        
        if (strncmp(line, "meta:width=", 11) == 0) {
            map->width = atoi(line + 11); // Use dynamic width
            // if (w != MAP_WIDTH) { ... }
        } else if (strncmp(line, "meta:height=", 12) == 0) {
            map->height = atoi(line + 12); // Use dynamic height
             /* if (h != MAP_HEIGHT) {
                fprintf(stderr, "FATAL: Map height mismatch. Expected %d, got %d in %s\n", MAP_HEIGHT, h, filename);
                fclose(f);
                exit(1);
            } */
        } else if (strncmp(line, "meta:name=", 10) == 0) {
            strncpy(map->name, line + 10, 63);
            map->name[63] = '\0';
        } else if (strncmp(line, "exit:", 5) == 0) {
            // Format: exit:x=53,y=8,file=PROCEDURAL,tx=-1,ty=-1
            // Simple parsing assuming strict format or robust enough
            if (map->exit_count < 16) {
                MapExit* e = &map->exits[map->exit_count++];
                // sscanf is risky with strings, but we control the format.
                // Or parse manually.
                
                // Let's use sscanf for simplicity if format is rigid.
                char file_buf[64] = {0};
                sscanf(line, "exit:x=%d,y=%d,file=%63[^,],tx=%d,ty=%d", 
                       &e->x, &e->y, file_buf, &e->target_x, &e->target_y);
                strcpy(e->target_file, file_buf);
            }
        } else if (strcmp(line, "layer:terrain") == 0) {
            in_terrain = true;
            continue;
        }
        
        if (in_terrain) {
            if (y >= map->height) continue; // Safety
            
            for (int x = 0; x < map->width && line[x] != 0; x++) {
                if (line[x] == '#') {
                    map->tiles[x][y].type = TILE_WALL;
                } else if (line[x] == '.') {
                    map->tiles[x][y].type = TILE_FLOOR;
                } else {
                    map->tiles[x][y].type = TILE_FLOOR; // Fallback
                }
            }
            y++;
        }
    }
    
    fclose(f);
}

// Field of View (Recursive Shadowcasting)

// Raycasting fallback (Simple, robust)
void map_compute_fov(Map* map, int px, int py, int radius) {
    // 1. Reset visibility
    for (int x = 0; x < map->width; x++) {
        for (int y = 0; y < map->height; y++) {
            map->tiles[x][y].visible = false;
        }
    }
    
    // 2. Mark player tile visible
    if (px >= 0 && px < map->width && py >= 0 && py < map->height) {
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
                
                if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) break;
                
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
                
                if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) break;
                
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
    for(int x=0; x<map->width; x++) {
        for(int y=0; y<map->height; y++) {
            if (map->smell[x][y] > 30) 
                 map->smell[x][y] -= 30;
            else 
                 map->smell[x][y] = 0;
        }
    }

    // 2. Source (Player)
    if (px >= 0 && px < map->width && py >= 0 && py < map->height) {
        map->smell[px][py] = 255;
    }

    // 3. Diffusion Pass (using temp buffer to avoid directional bias)
    uint8_t next_smell[MAX_MAP_WIDTH][MAX_MAP_HEIGHT];
    // Copy current state
    for(int x=0; x<map->width; x++) {
        for(int y=0; y<map->height; y++) {
            next_smell[x][y] = map->smell[x][y];
        }
    }

    uint8_t drop_off = 80; // Diffusion loss

    for(int x=1; x<map->width-1; x++) {
        for(int y=1; y<map->height-1; y++) {
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
    for(int x=0; x<map->width; x++) {
        for(int y=0; y<map->height; y++) {
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
    for(int x=0; x<map->width; x++) {
        for(int y=0; y<map->height; y++) {
            map->sound[x][y] = SOUND_NONE;
        }
    }

    if (px < 0 || px >= map->width || py < 0 || py >= map->height) return;

    // BFS
    // Max queue size = map size (overkill but safe stack usage)
    // Actually stack might be small? 54*16 = 864 structs. 
    // Struct is ~16 bytes. 13KB. Safe for stack.
    SoundNode queue[MAX_MAP_WIDTH * MAX_MAP_HEIGHT];
    int head = 0;
    int tail = 0;
    bool visited[MAX_MAP_WIDTH][MAX_MAP_HEIGHT] = {false};

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

            if (nx < 0 || nx >= map->width || ny < 0 || ny >= map->height) continue;
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
     if (x < 0 || x >= map->width || y < 0 || y >= map->height) return false;
     return map->smell[x][y] > 0;
}

SoundState map_sound_at(const Map* map, int x, int y) {
     if (x < 0 || x >= map->width || y < 0 || y >= map->height) return SOUND_NONE;
     return map->sound[x][y];
}

bool map_is_walkable(Map* map, int x, int y) {
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) return false;
    return map->tiles[x][y].type == TILE_FLOOR;
}

// ----------------------------------------------------------------------------
// Occupancy
// ----------------------------------------------------------------------------

void map_set_occupied(Map* map, int x, int y, bool occupied) {
    if (x < 0 || y < 0 || x >= map->width || y >= map->height) return;
    map->tiles[x][y].occupied = occupied;
}

bool map_is_occupied(Map* map, int x, int y) {
    if (x < 0 || y < 0 || x >= map->width || y >= map->height) return true; // Treat OOB as occupied
    return map->tiles[x][y].occupied;
}
