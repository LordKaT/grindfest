You are an external coding agent. Update wall auto-tiling so wall connectivity is computed only from tiles the player actually knows (visible or explored). This prevents wall glyphs at the FOV edge from leaking hidden structure.

## Current behavior (src/ui.c)
- Wall glyphs are auto-tiled using `wall_mask_at()` and `get_wall_glyph()` with a WACS_* table.
- `ui_render_map()` hides tiles that are neither visible nor explored and dims explored tiles.
- `wall_mask_at()` uses `tile_at()` to read raw map topology (`map->tiles[x][y].type`) even when those neighbors are NOT visible and NOT explored.
- `get_wall_glyph()` returns NULL for `mask == 0`, and the renderer falls back to `'X'`, even though table[0] is set to `WACS_BLOCK`.

## Bug / symptom
- Walls that are visible/explored can render as tees or pluses based on adjacent walls the player has not discovered.
- This looks wrong at the fog boundary and leaks information beyond FOV.

## Goal
Compute wall connectivity using only known tiles:
- A neighbor contributes to the wall mask only if the neighbor is in-bounds, is a wall, and is known (visible OR explored).
- Unknown tiles must not influence the wall mask.

## Definitions
- Known tile = `visible || explored`
- Unknown tile = `!visible && !explored`

## Implementation requirements
1) Do the fix in the rendering path in `src/ui.c`. Do not change map generation or FOV code.
2) Add a helper like:

```c
static bool tile_is_known_wall(const Map* map, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return false;
    const Tile* t = &map->tiles[x][y];
    if (!(t->visible || t->explored)) return false;
    return t->type == TILE_WALL;
}
```

3) Update `wall_mask_at()` to use `tile_is_known_wall()` for N/E/S/W.
4) Remove the debug fallback `'X'`. For `mask == 0`, return a consistent glyph (prefer `WACS_BLOCK` from the existing table) so unknown boundary walls do not disappear.
5) Keep current visibility rules:
   - Unknown tiles render as blank.
   - Explored but not visible tiles render dim (`A_DIM`), including walls.

## Acceptance criteria (manual test)
- At the FOV boundary, walls do not show tees/pluses based on hidden tiles.
- As the player explores adjacent tiles, wall glyphs can update to tees/pluses once those neighbors become known.
- No `'X'` debug glyph appears.

## Deliverable
- A patch to `src/ui.c` implementing known-only wall masking and proper `mask == 0` handling, with no changes to map generation or FOV logic.
