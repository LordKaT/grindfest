**Role:** You are a C Coding Agent.
**Task:** Implement Scrolling Camera, Dynamic Map Sizes, and a Test Environment.

**Context**
We are decoupling the "Map Size" from the "Viewport Size". Currently, the game hardcodes both to 54x16. We need to allow maps up to 256x256 while viewing them through the existing 54x16 viewport.

**1. Data Structures (`src/map.h`)**

* **Refactor Constants:**
* Rename `MAP_WIDTH` -> `MAX_MAP_WIDTH` (Set to 256).
* Rename `MAP_HEIGHT` -> `MAX_MAP_HEIGHT` (Set to 256).

* **Update Map Struct:**
* Add `int width;` and `int height;` to the `Map` struct.
* Ensure the `tiles`, `smell`, and `sound` arrays use the new `MAX_MAP_*` constants for allocation.

**2. Logic Updates: Dynamic Loops (Crucial)**
You must find and replace all loops that iterate over the map. They currently use the constants (`MAP_WIDTH`). They **must** now use the instance variables (`map->width`).

* **`src/map.c`:** Update loops in `map_generate_dungeon` (set width/height to 54/16 here for legacy support), `map_compute_fov`, `map_update_smell`, and `map_update_sound`.
* **`src/game.c`:** Update spawn loops in `game_init` (or `game_spawn_mobs` if present) to use `current_map.width` / `height`.
* **`src/ui.c`:** Update `wall_mask_at` and `tile_is_known_wall` bounds checks.

**3. Map Loader Update (`src/map.c`)**
Refactor `map_load_static` to handle dynamic sizes.

* **Parsing:** Read `meta:width` and `meta:height` into `map->width` and `map->height`.
* **Validation:**
* Remove the fatal `exit(1)` check that demands `w == MAP_WIDTH`.
* Replace with a safety check: `if (w > MAX_MAP_WIDTH) { Error; }`.

* **Exit Schema:** Ensure exit parsing aligns with existing `MapExit` struct in `map.h` and parsing logic (reading `exit:x=...` lines).

**4. Camera Logic (`src/ui.c`)**
Implement the scrolling view in `ui_render_map`.

* **Constants:** Use the existing `MAP_VIEW_WIDTH` (54) and `MAP_VIEW_HEIGHT` (16) defined in `ui.c` as the viewport size.
* **Camera Calculation:**
* `cam_x = player->x - (MAP_VIEW_WIDTH / 2)`
* `cam_y = player->y - (MAP_VIEW_HEIGHT / 2)`

* **Clamping:**
* `cam_x = max(0, min(cam_x, map->width - MAP_VIEW_WIDTH))`
* `cam_y = max(0, min(cam_y, map->height - MAP_VIEW_HEIGHT))`
* *Safety:* If `map->width < MAP_VIEW_WIDTH`, force `cam_x = 0`.

* **Rendering:**
* Loop `view_x` from 0 to `MAP_VIEW_WIDTH`.
* Draw tile at `map[cam_x + view_x][cam_y + view_y]`.
* Draw entities at `(entity->x - cam_x), (entity->y - cam_y)`.
* **Culling:** Do not draw entities if `screen_x` is outside the viewport range.

**5. Test Map & Init Override (`src/game.c`)**

* **Test Map (`data/maps/test_scroll.map`):**
* Size: 108x32.
* Metadata: `meta:width=108`, `meta:height=32`.
* Content: Solid `#` border *except* at `x=1,y=16`.
* **Exit:** Place an exit at `x=1,y=16` (West edge).
* `exit:x=1,y=16,file=bastok.map,tx=52,ty=8`
* **Important:** The tile at `1,16` must be `.` (Floor) so the player can step on it to trigger the exit.

* **Game Init:**
* In `game_init()`, force the game state to skip the menu for testing.
* `g_game.current_state = STATE_DUNGEON_LOOP;`
* `map_load_static(&g_game.current_map, "data/maps/test_scroll.map");`
* `g_game.player.x = 54; g_game.player.y = 16;` (Center of large map).
* `map_set_occupied(..., 54, 16, true);`

**Deliverables**

1. `src/map.h`: Renamed constants and updated struct.
2. `src/map.c`: Updated loader and all simulation loops.
3. `src/ui.c`: Camera clamping and relative rendering.
4. `src/game.c`: Init override and loop updates.
5. `data/maps/test_scroll.map`: The valid test file.
