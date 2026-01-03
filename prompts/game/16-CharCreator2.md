**Role:** You are a C Coding Agent.
**Task:** Implement a Modal Menu System (Status Screen) with a robust State Machine and Input hierarchy.

**Core Philosophy**
We are introducing a "Game State" vs "Menu State" distinction.

1. **Dungeon State:** The game world "ticks". Input advances time.
2. **Menu State:** The game world is **frozen**. Input affects the UI only.
3. **Global Cancel:** The `ESC` key is a context-sensitive "Back" button, not a hard quit.

**1. Input System Refactor (The "Escape" Logic)**
Refactor the input enum to support a generic "Cancel/Back" action.

* **`src/input.h`:** Rename `INPUT_ACTION_QUIT` to `INPUT_ACTION_CANCEL`.
* **`src/input.c`:** Map `ESC` (27) to `INPUT_ACTION_CANCEL`.
* **`src/game.c`:** In the input handling logic:
* If `current_state == STATE_MENU`: `INPUT_ACTION_CANCEL` closes the menu and returns to `STATE_DUNGEON_LOOP`.
* If `current_state == STATE_DUNGEON_LOOP`: `INPUT_ACTION_CANCEL` triggers a "Quit Confirmation" or sets `running = false` (for now, just quit, but via the *Cancel* action path).

**2. State Machine & Loop Isolation**
We must ensure no "Game Logic" runs while in a menu.

* **`src/game.h`:**
* Add `STATE_MENU` to `GameState`.
* Add `MenuType active_menu` (e.g., `MENU_STATUS`) to `Game` struct.

* **`src/game.c`:** Refactor the main loop in `game_run`:

```c
// Pseudo-code Structure
while (g_game.running) {
    if (g_game.current_state == STATE_DUNGEON_LOOP) {
        update_dungeon_loop();
    } else if (g_game.current_state == STATE_MENU) {
        update_menu_loop();
    }
    // ... other states
}

```

* **`update_menu_loop()`:**
* Does **NOT** process the `turn` event queue.
* Does **NOT** call `map_compute_fov`.
* Only calls `ui_render_menu()` and processes `ui_get_input()`.

**3. Data Stubs (The "Stub First" Approach)**
We lack the logic to calculate "Attack Power" or "Race Names" dynamically right now. Do not implement complex logic.

* Create helper functions in `src/entity.c` (and prototypes in `entity.h`) that return **hardcoded placeholders**:
* `const char* entity_get_race_name(RaceType r)` -> Returns "Hume", "Elvaan", etc., or "Unknown".
* `const char* entity_get_job_name(JobType j)` -> Returns "WAR", "MNK", etc.
* `int entity_get_derived_attack(const Entity* e)` -> Returns `e->base_stats.str * 2` (Placeholder).
* `int entity_get_derived_defense(const Entity* e)` -> Returns `e->base_stats.vit * 2` (Placeholder).
* `int entity_get_tnl(int level)` -> Returns `level * 100` (Placeholder).

**4. Rendering & Layout**

* **Window Management (`src/ui.c`):**
* Add `static WINDOW *win_menu;`.
* Implement `ui_open_menu()`: Allocates a window centered on screen.
* Math: `x = (SCREEN_WIDTH - MENU_WIDTH) / 2`, `y = (SCREEN_HEIGHT - MENU_HEIGHT) / 2`.
* Implement `ui_close_menu()`: Deletes window, sets pointer to NULL.

* **"Frozen" Map:**
* In `ui_render_menu()`, do **not** call `werase(win_map)` or `ui_render_map()`.
* Use `touchwin(win_map)` followed by `wrefresh(win_map)` to repaint the *existing* map buffer as the background, then render the menu window on top.

* **Content:** Use the stub helpers above to populate the Status window with Name, Job, Race, and Stats.

**Deliverables**

1. `src/input.h` / `src/input.c`: Refactor Quit -> Cancel.
2. `src/game.h` / `src/game.c`: State machine refactor and loop splitting.
3. `src/entity.h` / `src/entity.c`: Stub getters for Race/Job/Stats.
4. `src/ui.c`: Menu window management and centering logic.
