You are an external coding agent. Fix the wall masking in `src/ui.c` so missing corner/tee tiles are rendered correctly. Keep changes focused to the map draw path.

## Context / audit
- `ui_render_map` builds a `mask` per wall tile and maps it to `ACS_*` glyphs.
- Debug output shows many tiles rendered as `0` (mask 0). Some of those should be corners/tees based on neighboring wall segments.
- The current mask logic only looks at adjacent floor/void, so it misses corners that are implied by nearby wall lines.

## Required behavior (explicit rules)
When a wall tile currently ends up as mask `0` (or otherwise empty), override it using these rules based on **neighbor wall glyphs**:
1) If `ACS_VLINE` is left of the tile, and both `ACS_HLINE` above and `ACS_HLINE` below, and all other neighbors are empty or mask `0`, then render the tile as `ACS_LTEE`.
2) If `ACS_VLINE` is left of the tile, and `ACS_HLINE` is below, and all other neighbors are empty or mask `0`, then render the tile as `ACS_URCORNER`.
3) If `ACS_VLINE` is right of the tile, and `ACS_HLINE` is below, and all other neighbors are empty or mask `0`, then render the tile as `ACS_ULCORNER`.

## Implementation guidance
- Use a temporary buffer (mask or glyph grid) so you can inspect neighbor glyphs when fixing `mask==0` tiles.
- Keep using `chtype` for glyphs so `ACS_*` attributes are preserved.
- Do not change map generation; this is a render‑time masking fix.
- Remove or guard the debug `0` render once the fix is in place.

## Deliverables
- Code changes that implement the three rules above and eliminate the incorrect `0` tiles.
- Short summary of changes and why they fix the masking issue.
