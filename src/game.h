#ifndef GAME_H
#define GAME_H

#include "map.h"
#include "entity.h"

typedef enum {
    STATE_START_MENU,
    STATE_CHAR_CREATOR,
    STATE_DUNGEON_LOOP,
    STATE_MENU,
    STATE_GAME_OVER
} GameState;

typedef enum {
    RENDER_MODE_NORMAL,
    RENDER_MODE_SMELL,
    RENDER_MODE_SOUND
} RenderMode;

typedef struct {
    GameState current_state;
    RenderMode render_mode;
    bool running;
    
    Map current_map;
    Entity player;
    
    // We typically might have an array of entities for the level
    // For this scaffold, a simple array suffices
    Entity entities[100];
    int entity_count;
    
    // UI Messages log
    // ... handled in UI module typically, but Game might push strings
} Game;

extern Game g_game;

void game_init(void);
void game_run(void);
void game_cleanup(void);

// Helper to get entity by ID
Entity* game_get_entity(EntityID id);

#endif
