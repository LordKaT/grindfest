#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stdint.h>

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

typedef enum {
    SOUND_NONE,
    SOUND_CLEAR,
    SOUND_MUFFLED
} SoundState;

typedef enum {
    ZONE_CITY,
    ZONE_FIELD
} ZoneType;

typedef struct {
    int x, y;
    char target_map[64];
    int target_x, target_y;
} MapExit;

typedef struct {
    Tile tiles[MAP_WIDTH][MAP_HEIGHT];
    uint8_t smell[MAP_WIDTH][MAP_HEIGHT]; // 0-255 smell value
    SoundState sound[MAP_WIDTH][MAP_HEIGHT]; // Sound Propagation state
    
    ZoneType zone_type;
    MapExit exits[16];
    int exit_count;
} Map;

// Map Gen
void map_generate_dungeon(Map* map);
void map_load_static(Map* map, const char* filename);
bool map_is_walkable(Map* map, int x, int y);

// Occupancy
void map_set_occupied(Map* map, int x, int y, bool occupied);
bool map_is_occupied(Map* map, int x, int y);

// FOV
#define FOV_RADIUS 8
void map_compute_fov(Map* map, int px, int py, int radius);

// Sensory
void map_update_smell(Map* map, int px, int py);
void map_update_sound(Map* map, int px, int py, int radius);

// Helpers
bool map_is_smelly(const Map* map, int x, int y);
SoundState map_sound_at(const Map* map, int x, int y);

#endif
