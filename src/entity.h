#ifndef ENTITY_H
#define ENTITY_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_NAME_LEN 32

typedef enum {
    JOB_WARRIOR,
    JOB_MONK,
    JOB_THIEF,
    JOB_BLACK_MAGE,
    JOB_WHITE_MAGE,
    JOB_RED_MAGE,
    JOB_WORM,
    JOB_MAX
} JobType;

typedef enum {
    RACE_HUME,
    RACE_ELVAAN,
    RACE_TARUTARU,
    RACE_MITHRA,
    RACE_GALKA,
    RACE_WORM,
    RACE_MAX
} RaceType;

typedef enum {
    ENTITY_PLAYER,
    ENTITY_ENEMY
} EntityType;

typedef enum {
    AI_IDLE,
    AI_WANDER,
    AI_ENGAGED,
    AI_BURROWING,
    AI_WORM_TRAVEL
} AIState;

// Detection Flags
#define DETECT_SIGHT (1 << 0)
#define DETECT_SOUND (1 << 1)
#define DETECT_SMELL (1 << 2)

typedef struct {
    int str;
    int dex;
    int vit;
    int agi;
    int intel;
    int mnd;
    int chr;
} Attributes;

typedef struct {
    int hp;
    int max_hp;
    int mp;
    int max_mp;
    int tp;      // 0-3000
    int max_tp;  // Usually 3000
} Resources;

// Forward declaration for combat target
// We use IDs instead of pointers to avoid dangling pointer issues if an entity dies/respawns
typedef int EntityID; 

typedef struct {
    EntityID id;
    EntityType type; // Player or Enemy
    int x, y;
    char name[MAX_NAME_LEN];
    char symbol;
    int color_pair;
    
    RaceType race;

    JobType main_job;
    JobType sub_job; // Reserved for future

    Attributes stats;
    Resources resources;

    // Combat State
    bool is_engaged;
    EntityID target_id;
    int weapon_delay;     // Base delay for auto-attacks
    int weapon_damage;    // Base damage
    
    // Respawn Logic
    bool is_active;       // If false, it's a "tombstone" waiting to respawn
    long respawn_timer;   // Turns remaining until respawn (if !is_active)
    
    // AI / Stats
    int move_speed;       // Ticks per tile (Default 100)
    bool is_aggressive;
    uint8_t detection_flags;
    AIState ai_state;
    
    // Worm Specific
    bool is_burrowed;
    int burrow_dest_x;
    int burrow_dest_y;

} Entity;

#endif
