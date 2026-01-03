You are an external coding agent. Based on the current codebase, implement the features below. Keep changes minimal and focused.

## Current codebase notes
- src/map.c: map_generate_dungeon is a stub (fills walls + fixed test room).
- src/game.c: game_init manually fills the map with TILE_FLOOR; map_generate_dungeon is never called.
- src/ui.c: ui_render_map draws walls with '#' and floors with '.'.
- src/map.h: map_is_walkable returns true only for TILE_FLOOR.

## Features to build
1) Random dungeon generation on map load using Drunken Walk.
2) Render walls using single-line wall glyphs (ncurses ACS_*), including corners/angles for linking vertical and horizontal walls. Do not use '#' for walls.
3) Ensure all rooms/regions are reachable by digging tunnels to any disconnected areas.
4) Place the player at a random valid floor location (not inside a wall) after map generation.

## Implementation guidance
- Update map_generate_dungeon in src/map.c:
  - Start with all TILE_WALL.
  - Run a Drunken Walk from a random interior start, carving TILE_FLOOR until a target floor coverage (e.g., 35-45%).
  - Keep the outer border as walls.
  - Initialize tile fields (visible/explored/occupied) to false for all tiles.
- Connectivity:
  - Flood fill from the first floor tile.
  - For any disconnected floor region, dig a corridor to the nearest connected tile (straight-line carve is fine, or another short walk) so all floors become connected.
- Map load:
  - Replace the hardcoded floor fill in src/game.c with a call to map_generate_dungeon (either in game_init or right before entering STATE_DUNGEON_LOOP).
  - After generation, choose a random TILE_FLOOR for g_game.player.x/y.
- Wall rendering in src/ui.c:
  - Replace '#' wall glyphs with ACS_HLINE, ACS_VLINE, ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER, ACS_TTEE, ACS_BTEE, ACS_LTEE, ACS_RTEE, ACS_PLUS based on neighbor walls.
  - Keep floors as '.'.

## Constraints
- Keep changes localized to map generation, map load, and rendering.
- Build with make and ensure it compiles.

## Deliverables
- Code changes implementing the features above.
- Short summary of changes and rationale.
