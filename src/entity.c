#include <stdio.h>
#include <string.h>
#include "entity.h"
#include "ui.h" // For logging

void entity_add_exp(Entity* e, int amount) {
    if (e->type != ENTITY_PLAYER) return; // Simple for now
    
    // Check main job cap?
    if (e->job_levels[e->main_job] >= 75) return; // Cap at 75 (classic FFXI)
    
    e->job_exp[e->main_job] += amount;
    
    // Simple leveling curve: 100 EXP per level
    while (e->job_exp[e->main_job] >= 100) {
        e->job_exp[e->main_job] -= 100;
        e->job_levels[e->main_job]++;
        e->current_level = e->job_levels[e->main_job];
        
        ui_log("%s is now Level %d %s!", e->name, e->current_level, 
            e->main_job == JOB_WARRIOR ? "Warrior" : "Adventurer");
    }
}

bool entity_has_key_item(const Entity* e, KeyItemType ki) {
    if (ki < 0 || ki >= KI_MAX) return false;
    return e->key_items[ki] > 0;
}

void entity_add_status(Entity* e, StatusEffectType type, int duration, int power) {
    // Check existing
    for (int i=0; i < e->effect_count; i++) {
        if (e->effects[i].type == type) {
             // Overwrite if stronger or refresh duration
             // Simple rule: always overwrite for now
             e->effects[i].duration = duration;
             e->effects[i].power = power;
             return;
        }
    }
    
    // Add new
    if (e->effect_count < 16) {
        e->effects[e->effect_count].type = type;
        e->effects[e->effect_count].duration = duration;
        e->effects[e->effect_count].power = power;
        e->effect_count++;
    }
}

void entity_tick_status(Entity* e) {
    if (!e->is_active) return;
    
    for (int i=0; i < e->effect_count; i++) {
        if (e->effects[i].type != STATUS_NONE) {
            e->effects[i].duration--;
            
            if (e->effects[i].duration <= 0) {
                 // Expired
                 // ui_log("%s's effect wears off.", e->name); // Optional spam
                 
                 // Remove by swap with last
                 e->effects[i] = e->effects[e->effect_count - 1];
                 e->effect_count--;
                 i--; // Revisit this index since we swapped
            }
        }
    }
}

// Helpers Stubs
const char* entity_get_race_name(RaceType r) {
    switch (r) {
        case RACE_HUME: return "Hume";
        case RACE_ELVAAN: return "Elvaan";
        case RACE_TARUTARU: return "Tarutaru";
        case RACE_MITHRA: return "Mithra";
        case RACE_GALKA: return "Galka";
        case RACE_WORM: return "Worm";
        default: return "Unknown";
    }
}

const char* entity_get_job_name(JobType j) {
     switch (j) {
        case JOB_WARRIOR: return "WAR";
        case JOB_MONK: return "MNK";
        case JOB_THIEF: return "THF";
        case JOB_BLACK_MAGE: return "BLM";
        case JOB_WHITE_MAGE: return "WHM";
        case JOB_RED_MAGE: return "RDM";
        default: return "???";
    }
}

int entity_get_derived_attack(const Entity* e) {
    return e->current_stats.str * 2; // Stub
}

int entity_get_derived_defense(const Entity* e) {
    return e->current_stats.vit * 2; // Stub
}

int entity_get_tnl(int level) {
    return level * 100;
}
