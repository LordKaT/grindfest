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

#define IS_PLAYER_JOB(x) (x >= JOB_WARRIOR && x <= JOB_RED_MAGE)

typedef enum {
    RACE_HUME,
    RACE_ELVAAN,
    RACE_TARUTARU,
    RACE_MITHRA,
    RACE_GALKA,
    RACE_WORM,
    RACE_MAX
} RaceType;

#define IS_PLAYER_RACE(x) (x >= RACE_HUME && x <= RACE_GALKA)

typedef enum {
    NATION_NONE,
    NATION_BASTOK,
    NATION_SANDORIA,
    NATION_WINDURST,
    NATION_MONSTER,
    NATION_MAX,
} NationType;

#define IS_PLAYER_NATION(x) (x >= NATION_BASTOK && x <= NATION_WINDURST)

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
#define DETECT_MAGIC (1 << 3)

typedef enum {
    KI_ADVENTURER_CERTIFICATE,
    KI_SUBJOB_PERMIT,
    KI_AIRSHIP_PASS,
    KI_PALADIN_SCROLL,
    KI_MAX
} KeyItemType;

typedef enum {
    STATUS_NONE,
    STATUS_PROTECT,
    STATUS_POISON,
    STATUS_PARALYSIS,
    STATUS_WEAKNESS
} StatusEffectType;

typedef struct {
    StatusEffectType type;
    int duration; // Turns/Ticks
    int power;    // Magnitude
} StatusEffect;

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
    NationType nation;

    // Job persistence
    JobType main_job;
    JobType sub_job; // Reserved for future
    int current_level; // Cache of job_levels[main_job]
    int job_levels[JOB_MAX];
    int job_exp[JOB_MAX];

    Attributes base_stats;    // Permanent stats
    Attributes current_stats; // Calculated (Base + Job + Gear + Buffs)
    Resources resources;

    // Progression / State
    uint8_t key_items[KI_MAX]; // 0=Locked, 1=Owned
    EntityID claimed_by;       // -1 if unclaimed
    
    // Status Effects
    StatusEffect effects[16];
    int effect_count;

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

// Helper Functions
void entity_add_exp(Entity* e, int amount);
bool entity_has_key_item(const Entity* e, KeyItemType ki);
void entity_add_status(Entity* e, StatusEffectType type, int duration, int power);
void entity_tick_status(Entity* e);
void entity_init_stats(Entity* e, RaceType r, JobType j);

// Stubs
const char* entity_get_race_name(RaceType r);
const char* entity_get_job_name(JobType j);
int entity_get_derived_attack(const Entity* e);
int entity_get_derived_defense(const Entity* e);
int entity_get_tnl(int level);

#endif
