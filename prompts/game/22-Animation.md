**Role:** You are a C Coding Agent.
**Task:** Implement Animated Water tiles (`~`) with real-time effects in a turn-based system.

**Context**
We are adding visual flair. The map parser needs to recognize Water (`W`). The rendering engine needs to draw it as a wave (`~`) with cycling colors.
**Crucial Architecture Change:** To animate while the player is idle, the dungeon input loop must switch from "Blocking" to "Polling" (timeout). However, menus and text input must remain **Blocking** to prevent UI bugs.

**1. Input System Refactor (`src/ui.h`, `src/ui.c`)**
Refactor the input function to support variable blocking modes.

* **Signature Change:** Update `ui_get_input` to accept a timeout:
`int ui_get_input(char* input_buffer, int max_len, int timeout_ms);`
* **Implementation (`ui.c`):**
* Call `wtimeout(win_input, timeout_ms);` before `wgetch`.
* *Note:* `timeout_ms < 0` means blocking. `timeout_ms > 0` is non-blocking.

* **Safety Fix (`ui_get_string`):**
* Inside `ui_get_string`, explicitly call `wtimeout(win_input, -1);` before `wgetnstr` to ensure text entry never times out, regardless of previous states.

**2. Input Action Mapping (`src/input.h`, `src/input.c`)**

* **Enum:** Add `INPUT_ACTION_TIMEOUT` to `InputAction`.
* **Mapping:** In `input_handle_key`:
* Check against `ERR` (ncurses constant for timeout).
* If `key == ERR`, return `INPUT_ACTION_TIMEOUT`.

**3. Call Site Updates (`src/game.c`)**
Update all calls to `ui_get_input` to match the new signature.

* **Blocking Contexts (Pass `-1`):**
* `update_start_menu`
* `update_char_creator` (all steps)
* `update_menu_loop`

* **Dungeon Context (Pass `150`):**
* In `update_dungeon`, pass `150` (ms).
* **Loop Logic:** Inside the `while (!turn_taken)` loop:
* If `res.type == INPUT_ACTION_TIMEOUT`:
* Call `ui_tick_animation()` (see below).
* Call `ui_render_map(...)` to show the new frame.
* Call `ui_refresh()`.
* `continue;` (Restart loop, do **not** set `turn_taken`).

**4. Map Data & Logic (`src/map.h`, `src/map.c`)**

* **Enum:** Add `TILE_WATER` to `TileType`.
* **Loader:** In `map_load_static`, map char `'W'` to `TILE_WATER`.
* **Logic:**
* `map_is_walkable`: Return `false` for `TILE_WATER`.
* `map_compute_fov`: Ensure water is **Transparent** (does not block LOS). Only `TILE_WALL` should break the ray.

**5. Rendering & Visuals (`src/ui.c`)**

* **Colors:** In `ui_init`, define 3 Blue variants:
* `10`: Blue text on Black.
* `11`: Cyan text on Black.
* `12`: White text on Black (Sparkle).

* **State:** Add `static int animation_frame = 0;` and `void ui_tick_animation(void) { animation_frame++; }`.
* **Render Logic (`ui_render_map`):**
* If tile is `TILE_WATER`:
* **Check Render Mode:**
* If `RENDER_MODE_SMELL` or `RENDER_MODE_SOUND`: Draw as a standard floor tile (`.`) or solid block to prevent visual noise.
* If `RENDER_MODE_NORMAL`:
* Glyph: `'~'`
* **Math:** `phase = (x + y + (animation_frame / 2)) % 4` (Div by 2 slows it down slightly).
* Map phases 0-3 to the color pairs (e.g., 10, 11, 12, 11) to create a shimmering wave effect.

**6. Test Data (`data/maps/bastok.map`)**

* Edit `bastok.map` to include a small pool of water (replace some `.` with `W`) so the feature can be verified immediately upon loading the city.

**Deliverables**

1. `src/ui.h`/`.c`: Updated input signature and animation helpers.
2. `src/input.h`/`.c`: Timeout handling.
3. `src/game.c`: Updated input calls and loop handling.
4. `src/map.h`/`.c`: Water tile logic.
5. `data/maps/bastok.map`: Added water tiles.