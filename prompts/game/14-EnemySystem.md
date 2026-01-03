**Role:** You are a C Coding Agent.
**Task:** Implement a unified "Entity" system, an "Occupancy" system, and a "Tunnel Worm" enemy with specific AI behaviors.

**Mental Sandbox Simulation**

* *Occupancy:* If a worm burrows, it must call `map_set_occupied(x,y,false)`. When it emerges, `map_set_occupied(x,y,true)`. If the target tile is occupied, the worm waits.
* *Smell Tracking:* The worm is "Blind". It cannot see the player. If the player attacks the worm (Engage), the worm will try to move. It looks at the 4 neighbors. It picks the one with the highest smell value. If the smell is 0, it doesn't know where the player is (Loss of Aggro).
* *Timing:* "15 turns". Scheduler uses ticks. I will define `TICKS_PER_TURN = 100` to resolve the unit ambiguity.

**Context**
We are refactoring the codebase to support Enemies. Currently, we only have a Player. We need a unified `Entity` struct that handles both. We also need to implement our first "thinking" enemy, the Tunnel Worm, which has unique movement and sensory rules.

**1. Data Structure Updates (`src/entity.h`)**
Refactor the `Entity` struct to support both Player and Enemies.

* **Entity Type:** Add `typedef enum { ENTITY_PLAYER, ENTITY_ENEMY } EntityType;` and a `type` field to `Entity`.
* **Stats:** Add `int move_speed;` (Default: 100). This represents the tick cost to move one tile.
* **Jobs/Races:**
* Add `JOB_WORM` to `JobType`.
* Add `RACE_WORM` to `RaceType`.

* **AI Fields:**
* `bool is_aggressive;` (Does it attack on sight/sound/smell?)
* `uint8_t detection_flags;` (Bitmask: `DETECT_SIGHT`, `DETECT_SOUND`, `DETECT_SMELL`).
* `AIState ai_state;` (Enum: `AI_IDLE`, `AI_WANDER`, `AI_ENGAGED`, `AI_BURROWING`, `AI_WORM_TRAVEL`).

* **Worm Specifics:**
* `bool is_burrowed;` (Render flag).
* `int burrow_dest_x`, `burrow_dest_y`.

**2. Map Occupancy API (`src/map.c` / `src/map.h`)**
* Implement `void map_set_occupied(Map* map, int x, int y, bool occupied);`
* Implement `bool map_is_occupied(Map* map, int x, int y);`
* **Update Movement:** Update the player movement logic in `game.c` to check `map_is_occupied` before moving, and update the flag (set false on old tile, set true on new tile) upon successful move.

**3. AI Module (`src/ai.c` / `src/ai.h`)**
Create `void ai_take_turn(Entity* e, Map* map, Game* game);`. This is called by the scheduler when an Enemy's `EVENT_MOVE` triggers.

**Sensory Logic:**
* **Sight:** Do NOT use `tile.visible` (which is player-centric). Implement a helper `bool ai_can_see_target(Map* m, Entity* observer, Entity* target)` that casts a ray (Bresenham) to check for walls.
* **Smell:** Use the existing `map->smell` grid.
* **Aggro Logic:**
* If `is_aggressive` is true, check sensors (`detection_flags`).
* If SIGHT is on and `ai_can_see_target` is true -> Engage.
* If SMELL is on and `map->smell[x][y] > 0` -> Engage.

**4. The Tunnel Worm Implementation (`src/ai.c`)**
Implement specific logic for `JOB_WORM` / `RACE_WORM`.

* **Traits:**
* `is_aggressive = false` (Passive).
* `detection_flags = DETECT_SMELL`.
* `move_speed = 100` (Standard).

* **State Machine:**
1. **AI_IDLE:**
* Behavior: Wait for a random duration.
* Logic: Pick a duration `D = rand(15, 45)`. Calculate ticks: `ticks = D * 100`.
* Transition: Set state `AI_BURROWING`. Schedule next event for `current_time + ticks`.

2. **AI_BURROWING:**
* Behavior: Dig into the ground and pick a destination.
* Logic:
* Set `is_burrowed = true`.
* Free occupancy: `map_set_occupied(old_x, old_y, false)`.
* Pick a random `dest_x, dest_y` within radius 8 that is a valid floor AND `!map_is_occupied`.
* Calculate travel cost: `dist = ManhattanDist(old, new)`. `travel_time = dist * e->move_speed`.

* Transition: Set state `AI_WORM_TRAVEL`. Schedule next event for `current_time + travel_time`.

3. **AI_WORM_TRAVEL:**
* Behavior: Resurface.
* Logic:
* Set `x, y` to `burrow_dest`.
* Set `is_burrowed = false`.
* Claim occupancy: `map_set_occupied(x, y, true)`.

* Transition: Set state `AI_IDLE`. Schedule next event immediately (`current_time + 10`).

4. **AI_ENGAGED (Smell Tracking):**
* *Note:* Since `is_aggressive` is false, this state only happens if the Player explicitly attacks/engages the Worm (logic you will scaffold but not fully implement today).
* Logic: "True Blind" tracking.
* Scan 4 neighbors. Pick the one with the highest `map->smell` value.
* **Tie-breaker:** Randomly pick between tied neighbors.
* **Zero Smell:** If all neighbors are 0 (scent lost), set state `AI_IDLE` (give up).
* **Move:** If a valid tile is found, move there (update occupancy!). Schedule `current_time + move_speed`.

**5. Game Loop & Spawning (`src/game.c`)**
* **Spawning:**
* Remove the "Goblin" test code.
* Implement `game_spawn_worm()`:
* Find a random floor tile.
* Create Entity with `type=ENTITY_ENEMY`, `race=RACE_WORM`, `job=JOB_WORM`.
* Set ID `1` (Player is `0`). Update `entity_count`.
* **Crucial:** Call `map_set_occupied(x, y, true)` for the spawn point.

* **Update Loop:**
* In `update_dungeon`, when popping `EVENT_MOVE`:
* Check `e->type`.
* If `ENTITY_PLAYER`: Run existing input loop.
* If `ENTITY_ENEMY`: Call `ai_take_turn`.

**6. Rendering (`src/ui.c`)**
* In `ui_render_map`, check `e->is_burrowed`.
* If true, skip rendering that entity (do not draw it).

**Deliverables**

* Code changes for `entity.h`, `map.c/.h`, `ai.c/.h`, `game.c`, `ui.c`.
* **Do not implement combat mechanics** (damage/HP reduction). Focusing only on the AI state machine and movement/occupancy.
