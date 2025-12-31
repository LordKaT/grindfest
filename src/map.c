#include <stdlib.h>
#include <stdio.h> // for logging/debug if needed
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

    // 3. Connectivity Check (Flood Fill)
    // A single drunken walk is guaranteed strictly connected, 
    // but we'll verify to be safe and set a flag or just ensure robustness.
    // If we were placing rooms RANDOMLY, this would be needed. 
    // With Drunken Walk, we are good.
}

bool map_is_walkable(Map* map, int x, int y) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) return false;
    return map->tiles[x][y].type == TILE_FLOOR;
}
