**Role:** You are a C Coding Agent.
**Task:** Implement the full Character Creation Wizard (Name -> Race -> Job -> Nation) and a Static Map Loader for starting cities.

**Context**
We are replacing the temporary `update_char_creator` stub with a complete 4-step wizard. This concludes with loading a static city map instead of generating a random dungeon.
*Note:* The codebase currently lacks the state machine for this wizard. You must implement the enum and flow control logic from scratch.

**1. Data Structures (`src/entity.h`)**

* Add `NationType` enum:

```c
typedef enum {
    NATION_NONE,
    NATION_BASTOK,
    NATION_SANDORIA,
    NATION_WINDURST,
    NATION_MONSTER,
    NATION_MAX,
} NationType;

```

* Add `NationType nation;` to the `Entity` struct.
* Players are ONLY allowed to select between NATION_BASTOK, NATION_SANDORIA, and NATION_WINDURST.

**2. Character Creator State Machine (`src/game.c`)**
Refactor `update_char_creator` to handle a sequential wizard.

* **State Definition:** Define this enum in `game.c`:
```c
typedef enum {
    CREATOR_STEP_NAME,
    CREATOR_STEP_RACE,
    CREATOR_STEP_JOB,
    CREATOR_STEP_NATION
} CreatorStep;

```

* **State Management:** Add `static CreatorStep creator_step;` and `static int creator_selection;` to `game.c`.
* **Initialization:** When entering `STATE_CHAR_CREATOR`, set `creator_step = CREATOR_STEP_NAME`.
* **The Steps:**
1. **Name:** Use `ui_get_string` to set `player.name`. On confirm, go to `STEP_RACE`.
2. **Race:** List races. On confirm, set `player.race` and `base_stats` (per previous logic), go to `STEP_JOB`.
3. **Job:** List jobs. On confirm, set `player.main_job` and apply stat mods, go to `STEP_NATION`.
4. **Nation:** List the 3 nations (Bastok, San d'Oria, Windurst).
* **Action:** On confirm, set `player.nation`, call `game_load_city_map()`, and switch state to `STATE_DUNGEON_LOOP`.

**3. Static Map Loader (`src/map.c` / `src/map.h`)**
Implement `void map_load_static(Map* map, const char* filename);`.

* **File Structure:**
* Create directory `data/maps/` in the Makefile or assume it exists (add instruction to create it).

* **Format Rules:**
* Comments start with `%`. Ignore these lines anywhere.
* Metadata lines start with `meta:`.
* Terrain section starts with `layer:terrain`.
* **Crucial Parsing Logic:**
1. Read generic lines. If `layer:terrain` is found, switch to **Terrain Mode**.
2. **In Terrain Mode:** Treat `#` as `TILE_WALL` and `.` as `TILE_FLOOR`. Do **not** treat `#` as a comment here. Read exactly `MAP_HEIGHT` rows.

* **Validation:**
* If `meta:width` or `meta:height` do not match `MAP_WIDTH`/`MAP_HEIGHT`, **abort** with a fatal error log. Do not attempt to render a mismatched map.

**4. The Starting Map Files**
Create three files: `data/maps/bastok.map`, `data/maps/sandoria.map`, `data/maps/windurst.map`.

* **Content:**
```text
meta:width=54
meta:height=16
% This is a comment. The next section is data.
layer:terrain
######################################################
#....................................................#
#.........................###........................#
#.........................#.#........................#
#....................................................#
... (Fill remaining rows with floor/wall borders) ...

```

* *Note:* The center structure (`###` / `#.#`) ensures coordinate `27,8` is a valid floor tile (the dot inside the U).

**5. Game Loop & Spawning Updates (`src/game.c`)**

* **Remove Random Spawns:** Delete the code in `game_init` that loops to place the player and the code that spawns the "Worm". **Starting cities must be empty of enemies.**
* **City Loading Logic:**
* In the Nation selection confirm block, switch on `nation`:
* `NATION_BASTOK` -> load `data/maps/bastok.map`
* (etc...)

* **Player Spawn:** Manually set `g_game.player.x = 27` and `g_game.player.y = 8`.
* **Occupancy:** Call `map_set_occupied(..., 27, 8, true)`.

**Deliverables**

1. `src/entity.h`: Nation enum.
2. `src/game.c`: Full Wizard state machine and updated spawn logic (removing worms/random placement).
3. `src/map.c`: Robust `map_load_static` parser with strict mode switching.
4. `data/maps/*.map`: Three valid map files matching the dimensions.
