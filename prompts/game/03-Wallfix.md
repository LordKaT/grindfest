You are an external coding agent. Fix wall rendering to use the specified NetHack-style glyphs and nothing else.

## Problem
The current wall rendering uses random letters. Replace wall glyph selection so it only emits the characters below.

## Required wall glyphs
- Horizontal wall: `-`
- Vertical wall: `|`
- Corners: `┌` `┐` `└` `┘`
- Solid rock (if a distinct solid/undug wall is rendered): `#`

## Rules
- Choose a glyph based on neighboring wall tiles.
- Only use the glyphs listed above.
- If a wall tile cannot be strictly represented by the above set (e.g., T-junctions or 4-way intersections), render a blank space `' '` for now.

## Likely files
- src/ui.c (wall glyph selection in map rendering)
- src/map.c / src/map.h (tile types, if needed)

## Deliverables
- Code changes that enforce the glyph set and rules above.
- Short summary of what changed and why.
