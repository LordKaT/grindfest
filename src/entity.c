#include <stdio.h>
#include <string.h>
#include "entity.h"
#include <string.h>
#include "entity.h"
#include "ui.h" // For logging

static const Attributes RACE_BASE[RACE_MAX] = {
    //              STR, DEX, VIT, AGI, INT, MND, CHR
    [RACE_HUME]     = {8,  8,  8,  8,  8,  8,  8},
    [RACE_ELVAAN]   = {10, 8,  9,  7,  6,  9,  8},
    [RACE_TARUTARU] = {6,  8,  6,  8,  12, 8,  8},
    [RACE_MITHRA]   = {8,  10, 8,  10, 7,  7,  8},
    [RACE_GALKA]    = {9,  8,  12, 7,  6,  8,  7},
    [RACE_WORM]     = {8,  8,  8,  8,  8,  8,  8}
};

static const Attributes JOB_MODS[JOB_MAX] = {
    //                STR, DEX, VIT, AGI, INT, MND, CHR
    [JOB_WARRIOR]    = {3,  1,  3,  1,  0,  0,  0},
    [JOB_MONK]       = {2,  1,  4,  0,  0,  1,  0},
    [JOB_THIEF]      = {1,  4,  1,  4,  0,  0,  0},
    [JOB_BLACK_MAGE] = {0,  0,  1,  1,  5,  2,  1},
    [JOB_WHITE_MAGE] = {1,  0,  1,  1,  1,  5,  2},
    [JOB_RED_MAGE]   = {2,  2,  2,  2,  2,  2,  2},
    [JOB_WORM]       = {0,0,0,0,0,0,0}
};

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
void entity_init_stats(Entity* e, RaceType r, JobType j) {
    if (r < 0 || r >= RACE_MAX) r = RACE_HUME; // Safety
    if (j < 0 || j >= JOB_MAX) j = JOB_WARRIOR;
    
    // 1. Calculate Base
    e->base_stats.str = RACE_BASE[r].str + JOB_MODS[j].str;
    e->base_stats.dex = RACE_BASE[r].dex + JOB_MODS[j].dex;
    e->base_stats.vit = RACE_BASE[r].vit + JOB_MODS[j].vit;
    e->base_stats.agi = RACE_BASE[r].agi + JOB_MODS[j].agi;
    e->base_stats.intel = RACE_BASE[r].intel + JOB_MODS[j].intel;
    e->base_stats.mnd = RACE_BASE[r].mnd + JOB_MODS[j].mnd;
    e->base_stats.chr = RACE_BASE[r].chr + JOB_MODS[j].chr;
    
    // 2. Sync Current
    e->current_stats = e->base_stats;
    
    // 3. Resources
    e->resources.max_hp = (e->base_stats.vit * 5) + (e->base_stats.str * 2);
    
    if (j == JOB_WARRIOR || j == JOB_MONK || j == JOB_THIEF) {
        e->resources.max_mp = 0;
    } else {
        e->resources.max_mp = (e->base_stats.intel * 3) + (e->base_stats.mnd * 2);
    }
    
    // Fill
    e->resources.hp = e->resources.max_hp;
    e->resources.mp = e->resources.max_mp;
    
    // 4. Set Fields
    e->race = r;
    e->main_job = j;
    e->current_level = 1; 
    e->job_levels[j] = 1;
    e->job_exp[j] = 0;
}

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

const char* entity_get_race_name_short(RaceType r) {
    switch (r) {
        case RACE_HUME: return "HUM";
        case RACE_ELVAAN: return "ELV";
        case RACE_TARUTARU: return "TAR";
        case RACE_MITHRA: return "MIT";
        case RACE_GALKA: return "WAL";
        case RACE_WORM: return "WRM";
        default: return "Unknown";
    }
}

const char* entity_get_job_name(JobType j) {
     switch (j) {
        case JOB_WARRIOR: return "Warrior";
        case JOB_MONK: return "Monk";
        case JOB_THIEF: return "Thief";
        case JOB_BLACK_MAGE: return "Black MAge";
        case JOB_WHITE_MAGE: return "White Mage";
        case JOB_RED_MAGE: return "Red Mage";
        default: return "???";
    }
}

const char* entity_get_job_name_short(JobType j) {
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
