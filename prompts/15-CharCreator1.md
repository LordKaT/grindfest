Role: You are a C coding agent.
Task: Refactor the `Entity` struct to support persistent RPG progression (jobs/levels), key items, claim state, and status effects.

Context
The current `Entity` is a temporary snapshot. We need persistent progression mechanics inspired by FFXI (Job switching, Key Items, Claiming) while preserving deterministic turn-based behavior.

1) Split Stats: Base vs Current (`src/entity.h`)
- Rename `Attributes stats` -> `Attributes base_stats`.
- Add `Attributes current_stats`.
  - In FFXI, Current = (Base + JobMod + Gear + Buffs).
  - For this scaffold, initialize `current_stats = base_stats` in `game_init`.
- Refactor `src/combat.c` (and any other references) to read from `current_stats` instead of `stats`.

2) Job Persistence System
- Modify `Entity` struct in `src/entity.h`:

```c
int job_levels[JOB_MAX]; // Stores level for every job
int job_exp[JOB_MAX];    // Stores EXP for every job
int current_level;       // Caches job_levels[main_job] for easy access

```

* Logic Rules:
* When `main_job` changes, `current_level` must be updated.
* When gaining a level in the *active* job, `current_level` must update.

3. Key Items / Flags

* Add to `src/entity.h`:

```c
typedef enum {
    KI_ADVENTURER_CERTIFICATE,
    KI_SUBJOB_PERMIT,
    KI_AIRSHIP_PASS,
    KI_PALADIN_SCROLL,
    KI_MAX
} KeyItemType;

// Inside Entity struct:
uint8_t key_items[KI_MAX]; // 0 = Locked, 1 = Owned

```

* Ensure `<stdint.h>` is included.

4. Claim System (The "Purple Name" Mechanic)

* Add `EntityID claimed_by;` to `Entity`.
* Initialize to `-1` (Unclaimed).
* *Note:* Do not implement the rendering color change yet, just the data structure.

5. Status Effects

* Add to `src/entity.h`:

```c
typedef enum {
    STATUS_NONE,
    STATUS_PROTECT,
    STATUS_POISON,
    STATUS_PARALYSIS,
    STATUS_WEAKNESS
} StatusEffectType;

typedef struct {
    StatusEffectType type;
    int duration; // Measured in turns/ticks
    int power;    // e.g., Magnitude of Def bonus or DoT damage
} StatusEffect;

// Inside Entity struct:
StatusEffect effects[16]; // Fixed array for simplicity
int effect_count;

```

6. Helper Functions (`src/entity.c` + `src/entity.h`)
Create `src/entity.c` and implement:

* `void entity_add_exp(Entity* e, int amount);`
* Simple curve: Every 100 EXP = +1 Level.
* While `job_exp[main] >= 100`: decrement exp by 100, increment `job_levels[main]`.
* Update `current_level` if `main_job` was leveled.
* `bool entity_has_key_item(const Entity* e, KeyItemType ki);`
* `void entity_add_status(Entity* e, StatusEffectType type, int duration, int power);`
* Find first empty slot or overwrite existing status of same type.
* `void entity_tick_status(Entity* e);`
* Decrement duration of all active effects.
* Remove effects where `duration <= 0` (compact the array or mark as `STATUS_NONE`).

7. Initialization (`src/game.c`)

* Update `game_init` (player) and the enemy spawn block:
* Zero out `job_levels`, `job_exp`, `effects`, `key_items`.
* Player: Set `job_levels[main_job] = 1` and `current_level = 1`.
* Enemy: Set `job_levels[main_job]` (e.g., lvl 4 for Worm) and `current_level`.
* Set `claimed_by = -1`.
* Initialize `base_stats` and copy to `current_stats`.

8. Game Loop Integration (`src/game.c`)

* In `update_dungeon`, inside the `EVENT_MOVE` block for any entity:
* Call `entity_tick_status(e)` *before* processing input or AI to ensure effects age correctly.

Deliverables

* `src/entity.h` (updated struct and enums)
* `src/entity.c` (new logic implementation)
* `src/game.c` (initialization and loop update)
* `src/combat.c` (stat reference fix)
