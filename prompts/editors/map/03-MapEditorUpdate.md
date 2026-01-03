**Role:** You are a Full-Stack C and Python Developer.
**Task:** Upgrade the Map Editor to support "Smart Tiles" (Exits/Teleporters) and update the C Engine to support the new Teleport mechanic.

**Context**
We are introducing "Smart Tiles" where the map file header defines behavior (`exit`, `teleport`) linked to specific grid coordinates.

* **Python Editor:** Must allow placing 'Z' (Zone) and 'T' (Teleport) tiles, prompting for their metadata, and saving it to the file header.
* **C Engine:** Must parse these headers, render the new tiles, and trigger the teleport logic when the player steps on them.

### Part 1: Python Map Editor (`tools/map_editor.py`)

**1. Data Model Updates (`MapModel`)**

* **Trigger Storage:** Add `self.triggers = {}`.
* **Key:** Tuple `(x, y)`.
* **Value:** Dictionary `{'type': 'exit', ...}` or `{'type': 'teleport', ...}`.

* **Load Logic (`load_from_string`):**
* Parse lines starting with `exit:x=...,y=...` and `teleport:x=...,y=...`.
* Extract the `x,y` and store the full data dict in `self.triggers[(x,y)]`.
* **Preservation:** Ensure *unknown* metadata lines (not `width`, `height`, `name`, `exit`, `teleport`, or `layer`) are stored in a `self.extra_meta` list and written back on save.

* **Save Logic (`to_string`):**
* Before writing `layer:terrain`, iterate through `self.triggers`.
* Reconstruct the `exit:...` or `teleport:...` header string for each trigger.
* *Note:* This auto-deletes headers for tiles that have been erased from the grid.

**2. Controller Logic (`EditorController`)**

* **Smart Painting (`paint`):**
* **If placing 'Z' (Zone):**
* Prompt for: `Target Map` (string), `Target X` (int), `Target Y` (int).
* Store in `model.triggers`.

* **If placing 'T' (Teleport):**
* Prompt for: `Dest X` (int), `Dest Y` (int).
* Store in `model.triggers`.

* **If placing/erasing anything else:**
* Check `if (x,y) in model.triggers`. If yes, **delete** the trigger. (Replacing a Zone with a Floor removes the exit logic).

### Part 2: C Game Engine Updates

**1. Data Structures (`src/map.h`)**

* **Teleport Struct:**
```c
typedef struct {
    int x, y;               // Trigger Source
    int target_x, target_y; // Destination
} MapTeleport;

```

* **Map Update:**
* Define `#define MAX_TELEPORTS 16`.
* Add `MapTeleport teleports[MAX_TELEPORTS];` inside `Map`.
* Add `int teleport_count;`.

**2. Map Loader & Logic (`src/map.c`)**

* **Reset State:** In `map_load_static` AND `map_generate_dungeon`, strictly set `map->teleport_count = 0` before parsing.
* **Walkability:** Update `map_is_walkable` to **return true** if the tile type is `TILE_TELEPORT` (character 'T'). It must not be blocked.
* **Parsing (`map_load_static`):**
* Add parsing for `teleport:x=%d,y=%d,tx=%d,ty=%d`.
* Populate `map->teleports` array (check `teleport_count < MAX_TELEPORTS`).
* Map character `'T'` to `TILE_TELEPORT`.

**3. Visualization (`src/ui.c`)**

* **Color Pair:** In `ui_init`, define a new pair (e.g., ID 14) with `COLOR_MAGENTA` foreground (and `COLOR_BLACK` background).
* **Rendering:** Update `ui_render_map`:
* Case `TILE_ZONE`: Ensure it renders as is with `mvwadd_wchar(win_map, win_y, vx, 0x2591);`
* Case `TILE_TELEPORT`: Draw exactly as `TILE_ZONE` but using the Magenta color pair.

**4. Game Logic (`src/game.c`)**

* **Trigger Order:** In `update_dungeon`, the check order after a move must be:
1. Update Player X/Y & Occupancy.
2. Update Smell/Sound.
3. **Check Teleports.**
4. **Check Exits** (Only if no teleport happened).
5. Schedule next turn.

* **Teleport Implementation:**
* Iterate `current_map.teleports`.
* If `player.x == t.x && player.y == t.y`:
* **Validity Check:** Check `map_is_walkable(target_x, target_y)`.
* If false (Wall/Void): Log "The way is blocked!" and **abort** the teleport.

* **Occupancy Check:** If `map_is_occupied(target_x, target_y)`, log "Something is blocking the way!" and **abort**.
* **Execution:**
* Log "Teleporting..."
* `map_set_occupied(old_x, old_y, false)`
* Update `player.x/y` to targets.
* `map_set_occupied(new_x, new_y, true)`
* `map_compute_fov(...)` immediately so the screen doesn't flicker old data.
* **Return/Continue:** Do *not* process Exits this frame. Proceed to schedule the next turn event (cost = 100).

**Deliverables**

1. `tools/map_editor.py`: Updated with Trigger dictionary and I/O logic.
2. `src/map.h`: Constants and Structs.
3. `src/map.c`: Parsing, walkability, and reset logic.
4. `src/ui.c`: New colors and rendering.
5. `src/game.c`: Runtime trigger logic with collision checks.