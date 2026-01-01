**Role:** You are a C Coding Agent.
**Task:** Implement Map Transitions (Zoning) and the first Combat Field.

**Context**
We have static cities and a character creator. Now we need to let the player walk out of the city gates into a "Field" zone where monsters spawn. This involves parsing map exits, clearing game state between zones, and spawning enemies.

**1. Data Structures (`src/map.h`)**

* Add `ZoneType` enum:
```c
typedef enum {
    ZONE_CITY,  // Safe, Static
    ZONE_FIELD  // Dangerous, Procedural, Spawns Mobs
} ZoneType;

```

* Add to `Map` struct:
* `ZoneType zone_type;`
* `MapExit exits[16];`
* `int exit_count;`

* Define `MapExit`:
```c
typedef struct {
    int x, y;               // Trigger location
    char target_map[64];    // Filename "bastok.map" OR keyword "PROCEDURAL"
    int target_x, target_y; // Spawn location in new map (-1 for random)
} MapExit;

```

**2. Map Parser Update (`src/map.c`)**
Update `map_load_static` to parse exits.

* **Format:** `exit:x=53,y=8,file=PROCEDURAL,tx=-1,ty=-1`
* **Parsing Rules:**
* Read line-by-line.
* Ignore lines starting with `%` (Comments).
* Parse `exit:` lines into the `map->exits` array.
* Stop parsing metadata when `layer:terrain` is reached.
* **Strictness:** inside the terrain block, treat `#` as walls. Do not treat `#` as comments.

* **Defaults:** Static maps default to `ZONE_CITY`.

**3. State Management: Zoning (`src/game.c`)**
Create a helper `void game_transition_zone(const char* target_map, int tx, int ty);`

* **Logic:**
1. **Clear State:** Set `g_game.entity_count = 1` (Remove all mobs, keep Player).
2. **Clear Turns:** `turn_init()` to wipe pending events.
3. **Load Map:**
* If `target_map` == "PROCEDURAL":
* `map_generate_dungeon(...)`
* `current_map.zone_type = ZONE_FIELD`
* Spawn Mobs (see below).
* Place player at random valid location (if `tx == -1`).

* Else:
* `map_load_static(..., "data/maps/%s", target_map)` (Prepend path).
* `current_map.zone_type = ZONE_CITY`
* Place player at `tx, ty`.

4. **Re-occupy:** Update `map_set_occupied` for the player's new position.
5. **Schedule:** Add initial `EVENT_MOVE` for player to restart the loop.

**4. Mob Spawning (`src/game.c`)**
Since `data_load_monsters` is not ready, implement a temporary hardcoded spawner.

* Implement `void game_spawn_mobs(void)`:
* Loop 10 times.
* Create a temporary entity template (e.g., "Rabbit", HP: 20, Aggro: False, Symbol: 'r').
* Find a valid, unoccupied floor tile (`map_is_walkable` && `!map_is_occupied`).
* Add to `g_game.entities`.
* Ensure `type = ENTITY_ENEMY`.
* Schedule their first `EVENT_MOVE`.

**5. Zone Transition Hooks (`src/game.c`)**

* In `update_dungeon`, immediately *after* the player successfully updates their `x,y` (and occupancy):
* Iterate through `current_map.exits`.
* If `player.x == exit.x && player.y == exit.y`:
* Log "Zoning to %s...", exit.target_map
* Call `game_transition_zone(...)`.
* `return` immediately (do not continue processing the current frame).

**6. Data Updates (`data/maps/*.map`)**

* Update `bastok.map`, `sandoria.map`, `windurst.map`.
* **Add Exit:** `exit:x=53,y=8,file=PROCEDURAL,tx=-1,ty=-1` (Adjust coordinates if needed).
* **Fix Geometry:** Ensure the tile at `53,8` (or wherever you put the exit) is a `.` (Floor), NOT a `#` (Wall). The player must be able to step *onto* it.

**Deliverables**

1. `src/map.h`/`.c`: Updated struct and parser.
2. `src/game.c`: Transition logic, state clearing, and hardcoded mob spawner.
3. Updated `.map` files with valid exits and floor tiles at exit coordinates.
