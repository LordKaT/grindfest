**Role:** You are a C Coding Agent.
**Task:** Implement the Character Creation Wizard (Name -> Race -> Job -> Stats).

**Context**
We are replacing the current stubbed `update_char_creator` with a fully interactive menu system. This system must guide the user through three distinct steps, rendering specific menus for each, and finally calculating the player's starting stats based on FFXI rules (Race Base + Job Mod).

**1. State Management (`src/game.c`)**
Refactor `update_char_creator` to handle a sub-state machine.

* Define an enum (locally or in `game.h` if preferred):

```c
typedef enum {
    CREATOR_STEP_NAME,
    CREATOR_STEP_RACE,
    CREATOR_STEP_JOB
} CreatorStep;

```

* Add `CreatorStep creator_step;` and `int creator_selection;` to the `Game` struct (or manage locally if static).
* **Reset Logic:** When entering `STATE_CHAR_CREATOR`, set `step = NAME`, `selection = 0`.

**2. Input Handling (`src/input.c` / `src/input.h`)**

* Ensure `INPUT_ACTION_CONFIRM` (Enter key) is defined. Map `\n` or `KEY_ENTER` to it.
* Ensure Up/Down arrow keys are mapped to `INPUT_ACTION_MOVE_UP` / `DOWN`.

**3. The Wizard Steps**

* **Step 1: Name Input**
* **UI:** Clear screen. Print "Enter Name: " in the center.
* **Logic:** Use `ui_get_string` (blocking is fine for this specific step) to populate `g_game.player.name`.
* **Transition:** On Enter, go to `CREATOR_STEP_RACE`.

* **Step 2: Race Selection**
* **UI Layout:**
* **Left Window (Map View):** List all playable races (Hume, Elvaan, Tarutaru, Mithra, Galka).
* **Right Window (Panel):** Show the description of the *currently selected* race.

* **Visuals:** The selected line in the left window must be highlighted (White background, Black text).
* **Logic:** `UP/DOWN` updates `creator_selection`. `CONFIRM` sets `g_game.player.race` and advances to `CREATOR_STEP_JOB`.
* *Note:* Filter out `RACE_WORM` or `RACE_MAX`.

* **Step 3: Job Selection**
* **UI Layout:** Similar to Race. List Jobs (WAR, MNK, THF, BLM, WHM, RDM).
* **Right Window:** Show job description (e.g., "Standard tank/damage dealer").
* **Logic:** `UP/DOWN` updates `creator_selection`. `CONFIRM` sets `g_game.player.main_job`, calculates final stats, and moves to `STATE_DUNGEON_LOOP`.

**4. Stat Calculation Logic (`src/entity.c`)**
Implement the FFXI "Base + Job Mod" system.

* **Data Structures:** Define these static lookup arrays in `entity.c`:

```c
// Base stats for each race (STR, DEX, VIT, AGI, INT, MND, CHR)
static const Attributes RACE_BASE[RACE_MAX] = {
    [RACE_HUME]     = {8, 8, 8, 8, 8, 8, 8},
    [RACE_ELVAAN]   = {10, 8, 9, 6, 6, 9, 8},
    [RACE_TARUTARU] = {6, 8, 6, 8, 12, 8, 8},
    [RACE_MITHRA]   = {8, 10, 8, 10, 7, 7, 8},
    [RACE_GALKA]    = {9, 8, 12, 7, 6, 8, 7},
    [RACE_WORM]     = {8, 8, 8, 8, 8, 8, 8} // Fallback
};

// Modifiers for each job
static const Attributes JOB_MODS[JOB_MAX] = {
    [JOB_WARRIOR]    = {3, 1, 3, 0, 0, 0, 0},
    [JOB_MONK]       = {2, 1, 4, 0, 0, 0, 0},
    [JOB_THIEF]      = {1, 4, 1, 4, 0, 0, 0},
    [JOB_BLACK_MAGE] = {0, 0, 1, 1, 5, 2, 0},
    [JOB_WHITE_MAGE] = {1, 0, 1, 1, 1, 5, 2},
    [JOB_RED_MAGE]   = {2, 2, 2, 2, 2, 2, 2},
    [JOB_WORM]       = {5, 5, 5, 5, 5, 5, 5} // Fallback
};

```

* **Implementation of `entity_init_stats(Entity* e, RaceType r, JobType j)`:**
1. **Calculate Base:** `e->base_stats = RACE_BASE[r] + JOB_MODS[j]` (Sum each field).
2. **Sync Current:** `e->current_stats = e->base_stats`.
3. **Derive Resources:** Calculate Max HP and MP using the stats (since they aren't in the arrays):
* `e->resources.max_hp = (e->base_stats.vit * 5) + (e->base_stats.str * 2);`
* `e->resources.max_mp = (e->base_stats.intel * 3) + (e->base_stats.mnd * 2);`
* *Exception:* If Job is WAR, MNK, or THF, set `max_mp = 0` (No MP pool).
4. **Fill Pools:** Set `hp = max_hp` and `mp = max_mp`.

**5. Rendering Updates (`src/ui.c`)**

* Create `ui_render_creator_menu(const char* title, const char** items, int count, int selection, const char* description)`.
* This generic function can be used for both Race and Job steps.
* It renders the list in `win_map` and the description in `win_panel`.
* Handle highlighting attributes (`wattron(A_REVERSE)`).

**Deliverables**

1. `src/game.c`: Implemented wizard state machine.
2. `src/ui.c`: New menu rendering function.
3. `src/input.c`: Confirmation key mapping.
4. `src/entity.c`: Stat lookup tables and initialization logic.
5. `src/entity.h`: Prototype for `entity_init_stats`.