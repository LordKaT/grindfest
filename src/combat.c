#include <stdio.h>
#include "combat.h"
#include "turn.h"
#include "ui.h" // For logging

void combat_engage(Entity* attacker, EntityID target_id) {
    if (attacker->is_engaged && attacker->target_id == target_id) {
        ui_log("%s is already engaged!", attacker->name);
        return;
    }

    attacker->is_engaged = true;
    attacker->target_id = target_id;
    
    // Set default delay if 0
    if (attacker->weapon_delay <= 0) attacker->weapon_delay = 100;
    
    ui_log("%s engages target!", attacker->name);
    
    // Schedule first attack immediately
    // FFXI: You engage, then delay starts filling
    // Inverse (monster->player) is true as well
    turn_add_event(turn_get_current_time() + attacker->weapon_delay, attacker->id, EVENT_ATTACK_READY);
}

void combat_disengage(Entity* attacker) {
    if (!attacker->is_engaged) return;
    attacker->is_engaged = false;
    ui_log("%s disengages.", attacker->name);
    // Note: Pending Attack events might still pop.
    // The handler must check `is_engaged` before executing.
}

void combat_execute_auto_attack(Entity* attacker, Entity* target) {
    if (!attacker || !target) return;
    
    int damage = (attacker->current_stats.str > 0 ? attacker->current_stats.str : 10) + attacker->weapon_damage;
    // Stub calculation
    
    target->resources.hp -= damage;
    if (target->resources.hp < 0) target->resources.hp = 0;
    
    // Gain TP
    attacker->resources.tp += 100; // Fixed 100 TP
    if (attacker->resources.tp > 3000) attacker->resources.tp = 3000;
    
    ui_log("%s hits %s for %d dmg. TP: %d", attacker->name, target->name, damage, attacker->resources.tp);
    
    if (target->resources.hp == 0) {
        ui_log("%s defeats %s!", attacker->name, target->name);
        combat_disengage(attacker);
        
        target->is_active = false;
        // Respawn needs to happen at map spawn points. Let's mark as inactive for now.
    }
}
