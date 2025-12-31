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

// ----------------------------------------------------------------------------
// Field of View (Recursive Shadowcasting)
// ----------------------------------------------------------------------------

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

bool map_is_walkable(Map* map, int x, int y) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) return false;
    return map->tiles[x][y].type == TILE_FLOOR;
}
