You are an external coding agent. Modify map generation to remove isolated wall segments by converting them to floors in a post-generation pass. Keep changes focused and explain the fix.

## Context
- Map generation lives in `src/map.c` (`map_generate_dungeon`).
- Rendering in `src/ui.c` treats walls as connected segments; isolated wall tiles show up as visual artifacts.

## Required change
1) Add a post-generation cleanup pass after dungeon carving that scans all tiles and finds isolated wall tiles.
2) An isolated wall tile is one where all 4 cardinal neighbors are NOT walls (or are out-of-bounds).
3) Flip isolated wall tiles from `TILE_WALL` to `TILE_FLOOR`.

## Constraints
- Keep the core generation algorithm (Drunken Walk) intact.
- Only modify `src/map.c` (and `src/map.h` if you need helper declarations).
- Do not change rendering logic in `src/ui.c`.

## Deliverables
- Code changes implementing the post-generation cleanup pass.
- Short summary of what changed and why.
