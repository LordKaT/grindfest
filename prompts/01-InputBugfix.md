You are an external coding agent. Fix the bugs below in this repo. Keep changes minimal and focused.

## Bugs to fix
1) Movement does not include diagonal movement. Add diagonal movement support. Use numpad digits 7, 9, 1, 3 for diagonals and keep existing 8, 2, 4, 6. Do not add letter or arrow movement.
2) Moving the character causes the background color to extend to the right when moving far enough right. Fix this by ensuring the background is drawn BEFORE the character is moved.
3) The character should only be drawn AFTER the entire map is drawn. The proper render order is:
   - Map
   - Objects
   - NPC
   - Enemies
   - Player
   - UI
   Do not interleave these render passes. Keep them separate and in order.

## Notes
- Likely files: src/input.c, src/input.h, src/game.c, src/ui.c.
- Build with `make` and ensure it compiles.

## Deliverables
- Code changes that fix the bugs.
- A short summary of what you changed and why.
