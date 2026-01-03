#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_MAP_WIDTH 256
#define MAX_MAP_HEIGHT 256

typedef enum {
    TILE_EMPTY = 0,
    TILE_FLOOR,
    TILE_WALL,
    TILE_DOOR,
    TILE_WATER,
    TILE_BRIDGE,
    TILE_ZONE,       // Blank/Empty but acts as a wall. Specifically for "Zoning" between maps (visual flair, honestly)
    TILE_VOID,       // Blank/Empty but does NOT act as a wall. Visual flair only.
    TILE_STAIRS_UP,
    TILE_STAIRS_DOWN,
    TILE_TELEPORT
} TileType;

typedef struct {
    TileType type;
    bool visible;   // In FOV
    bool explored;  // Seen before
    bool seen;      // (Legacy/Transient)
    bool occupied;
} Tile;

// Zoning Metadata
typedef struct {
    int x, y;
    char target_file[64];
    int target_x, target_y;
} MapExit;

typedef struct {
    int x, y;
    int target_x, target_y;
} MapTeleport;


typedef enum {
    SOUND_NONE = 0,
    SOUND_CLEAR = 1,
    SOUND_MUFFLED = 2
} SoundState;

typedef struct {
    char name[64];
    int width;
    int height;
    
    Tile tiles[MAX_MAP_WIDTH][MAX_MAP_HEIGHT];
    int smell[MAX_MAP_WIDTH][MAX_MAP_HEIGHT];    // 0 = None, 255 = Fresh
    SoundState sound[MAX_MAP_WIDTH][MAX_MAP_HEIGHT];
    
    MapExit exits[256];
    int exit_count;
    #define MAX_TELEPORTS 16
    MapTeleport teleports[MAX_TELEPORTS];
    int teleport_count;
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
