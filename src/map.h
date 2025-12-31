#ifndef MAP_H
#define MAP_H

#include <stdbool.h>

#define MAP_WIDTH 54
#define MAP_HEIGHT 16

typedef enum {
    TILE_WALL,
    TILE_FLOOR,
    TILE_VOID
} TileType;

typedef struct {
    TileType type;
    bool visible;   // Currently in FOV
    bool explored;  // Seen before (fog of war)
    bool occupied;  // Is an entity standing here?
} Tile;

typedef struct {
    Tile tiles[MAP_WIDTH][MAP_HEIGHT];
} Map;

// Map Gen
void map_generate_dungeon(Map* map);
bool map_is_walkable(Map* map, int x, int y);

// FOV
#define FOV_RADIUS 8
void map_compute_fov(Map* map, int px, int py, int radius);

#endif
