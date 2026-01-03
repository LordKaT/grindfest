You are an external coding agent. Implement field-of-view (FOV) rendering and visibility tracking in this codebase.

## Goals
- Only tiles within the player’s FOV are rendered at full brightness.
- Tiles previously seen (explored) are rendered in a darker color.
- Enemies are drawn only when in FOV; if not in FOV, they are not drawn.

## Codebase audit (what exists)
- `src/map.h` defines `Tile` with `visible` and `explored` booleans, but they are never updated.
- `src/ui.c` renders all tiles and all entities unconditionally.
- `src/game.c` renders every turn but never computes visibility.

## Required changes
1) Add FOV computation (new helper(s) in `src/map.c` + declarations in `src/map.h`).
   - Add `map_clear_visibility(Map*)` and `map_compute_fov(Map*, int px, int py, int radius)` (names are flexible).
   - FOV should treat `TILE_WALL` as opaque (blocks sight). `TILE_FLOOR` should be transparent.
   - Use a simple approach that fits the small map size: per‑tile LOS using Bresenham or a basic shadowcasting implementation.
   - Use a fixed radius constant (e.g., `#define FOV_RADIUS 8` in `map.h` or `game.h`).
   - For each visible tile, set `tile.visible = true` and `tile.explored = true`.
   - Tiles not in FOV should have `tile.visible = false` (but keep `explored` true if they were seen before).

2) Call FOV update in the game loop.
   - In `src/game.c` (inside the dungeon loop), compute FOV before `ui_render_map` each render.
   - Use the player’s current `x/y` position.

3) Update rendering in `src/ui.c`.
   - Only draw tiles if `tile.visible` or `tile.explored` is true.
   - If `tile.visible`: draw normally.
   - If `tile.explored` but not visible: draw in a darker style (use `A_DIM` or a dim color pair).
   - Entities (NPCs/enemies) should only be drawn if their tile is currently visible.
   - Player should always be drawn.

## Notes
- Keep changes localized to visibility/FOV and rendering.
- Reuse the existing `visible`/`explored` fields instead of adding new structures unless necessary.
- Ensure FOV does not read out-of-bounds on map edges.

## Deliverables
- Code implementing FOV and visibility tracking.
- Brief summary of changes and why they satisfy the goals.
