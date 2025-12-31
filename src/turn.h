#ifndef TURN_H
#define TURN_H

#include "entity.h"

// Priority Queue Scheduler

typedef enum {
    EVENT_MOVE,
    EVENT_ATTACK_READY, // The moment an auto-attack swing happens
    EVENT_RESPAWN_TICK  // Check for respawns (could be global or per entity)
} EventType;

typedef struct {
    long time;           // Absolute game time
    EntityID entity_id;
    EventType type;
    long priority_id;    // Tie-breaker for insertion order
} GameEvent;

void turn_init(void);
void turn_add_event(long time, EntityID entity_id, EventType type);
GameEvent turn_pop_event(void);
bool turn_queue_is_empty(void);
long turn_get_current_time(void);

#endif
