You are an external coding agent. Fix the missing wall segments in the dungeon map by auditing both map creation and rendering. Keep changes focused and explain the exact bug(s) you are addressing.

## Findings (root causes)
- `src/map.c` generates a drunk‑walk map by carving floors out of a solid wall mass. That means most wall tiles have 3–4 wall neighbors.
- `src/ui.c` chooses wall glyphs based on **adjacent wall tiles** and explicitly renders T‑junctions / 4‑way intersections as a blank space. In a wall‑mass map, this produces large black gaps, even between corner pieces.

## Required fixes
1) Render walls based on **adjacent floor/void tiles**, not adjacent wall tiles.
   - A wall tile should be drawn as a line only if it borders at least one floor (or void/outside) tile.
   - This converts the wall mass into an outline around floor regions, which removes the bogus T‑junction blanks.
2) Use ncurses line drawing (`ACS_*`) in the map, consistent with the UI box.
   - Store glyphs in a `chtype`, not `char`.
3) Eliminate black gaps for wall tiles that should be visible.
   - If a wall tile has **no adjacent floor/void**, render it as solid rock `#` (allowed by prior prompt).
   - If adjacency is ambiguous (3–4 open sides), prefer `#` over `' '` so walls don’t disappear.

## Concrete mapping (based on adjacent FLOOR/VOID)
Let N/S/E/W indicate neighboring tiles that are floor (or void/outside):
- E or W (or E+W): `ACS_VLINE`
- N or S (or N+S): `ACS_HLINE`
- S+E: `ACS_ULCORNER`
- S+W: `ACS_URCORNER`
- N+E: `ACS_LLCORNER`
- N+W: `ACS_LRCORNER`
- 0 neighbors: `#`
- 3–4 neighbors: `#`

## Scope
- Primary changes in `src/ui.c` (`ui_render_map`).
- Only adjust `src/map.c` if you need to tag void/outside tiles for the adjacency checks.
- Do not alter UI layout or box rendering.

## Deliverables
- Code changes that remove the missing wall segments and keep wall glyphs consistent.
- Short summary of what changed and why.
