#ifndef COMBAT_H
#define COMBAT_H

#include "entity.h"

void combat_engage(Entity* attacker, EntityID target_id);
void combat_disengage(Entity* attacker);
void combat_execute_auto_attack(Entity* attacker, Entity* target);

#endif
