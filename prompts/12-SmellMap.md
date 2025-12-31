### Prompt for Coding Agent

Role: You are a C coding agent.
Task: Implement two new sensory map layers (Smell and Sound) and a debug visualization system.

Context:
In Final Fantasy XI, aggro is determined by Sight, Sound, and Smell. We need to scaffold Sound and Smell now so future AI can query `map_is_smelly(x,y)` or `map_is_loud(x,y)`.

1) Data Structures
Extend `Map` in `src/map.h` to include two parallel 2D arrays (not inside `Tile`):
- Smell map: `uint8_t smell[MAP_WIDTH][MAP_HEIGHT]` (0..255).
- Sound map: `SoundState sound[MAP_WIDTH][MAP_HEIGHT]` where:
  - `SOUND_NONE`, `SOUND_CLEAR`, `SOUND_MUFFLED`.

Add a `RenderMode` state (normal/smell/sound). Choose ONE place to store it:
- Option A: `Game` struct in `src/game.h` (preferred), or
- Option B: a static in `src/ui.c`.
Be explicit and keep it consistent.

2) Smell System (persistent)
Implement `map_update_smell(Map* map, int px, int py)` called every turn.

Steps:
1. Global decay: subtract a constant (e.g., 20) from every cell, clamp at 0.
2. Source: set the player tile to 255.
3. Diffusion: perform a separate pass using a temporary buffer so order does not bias results.
   - For each FLOOR tile, compare to 4 neighbors (N/E/S/W).
   - New value: `max(current, neighbor - diffusion_loss)`.
   - Walls remain 0 and do not receive or transmit smell.

3) Sound System (instantaneous)
Implement `map_update_sound(Map* map, int px, int py, int radius)` called on player movement (only after a successful move).

Rules:
- Clear the sound map to `SOUND_NONE` each update.
- BFS from player position.
- Track `distance` (<= radius) and `walls_penetrated`.
- When stepping into a WALL tile, increment `walls_penetrated` by 1.
- Allow propagation through at most 1 wall; if `walls_penetrated >= 2`, stop.
- Mark tiles:
  - `walls_penetrated == 0` -> `SOUND_CLEAR`
  - `walls_penetrated == 1` -> `SOUND_MUFFLED`
- Do not propagate out of bounds.

4) Render Mode Toggle (debug)
Add input actions:
- `INPUT_ACTION_VIEW_NORMAL`
- `INPUT_ACTION_VIEW_SMELL`
- `INPUT_ACTION_VIEW_SOUND`

Update `input_handle_key` in `src/input.c` to map `KEY_F(1..3)` to those actions.
In the dungeon input loop in `src/game.c`, when these actions are returned, update the render mode state.

5) Rendering (src/ui.c)
Add a render mode switch inside `ui_render_map`:
- Normal mode: current rendering unchanged.
- Smell mode:
  - Walls draw normally.
  - Floors draw a solid block (`WACS_BLOCK` or `ACS_BLOCK`).
  - Color: use a red color pair. Use `A_BOLD` for smell >= 128, `A_NORMAL` for 64..127, `A_DIM` for 1..63. Smell 0 draws as normal floor.
- Sound mode:
  - Walls draw normally.
  - Floors:
    - `SOUND_CLEAR`: blue solid block (`WACS_BLOCK`) + `A_BOLD`.
    - `SOUND_MUFFLED`: cyan `WACS_CKBOARD` (or `#` if needed) + `A_DIM`.
    - `SOUND_NONE`: normal floor.

Ensure color pairs exist in `ui_init` (add blue and cyan pairs).

6) Initialization and helpers
- Initialize smell/sound arrays in `map_generate_dungeon` (zero them).
- Add helpers (optional but recommended):
  - `bool map_is_smelly(const Map* map, int x, int y)` (smell > 0)
  - `SoundState map_sound_at(const Map* map, int x, int y)`

7) Performance constraints
- No per-frame mallocs. If you use a temp smell buffer or BFS queue, allocate on stack or use static arrays sized to MAP_WIDTH*MAP_HEIGHT.

Deliverables
1) `map.c` / `map.h` changes for data, smell/sound updates, and any helpers.
2) `game.c` changes to call smell every turn and sound after a successful move.
3) `input.c` / `input.h` updates for F-keys.
4) `ui.c` updates for render mode and colors.
5) Brief summary of changes and rationale.
