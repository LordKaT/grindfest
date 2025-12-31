You are an external coding agent. Fix the wall glyphs in the dungeon map so they render correctly (matching the UI box lines). Use ncurses `ACS_*` glyphs consistently for walls, just like the UI box uses ncurses line drawing, and explain the specific bug you are fixing.

## Diagnosis (root cause)
In `src/ui.c` `ui_render_map` uses `char glyph` but assigns `ACS_*` constants (e.g., `ACS_ULCORNER`). Those are `chtype` values that include the `A_ALTCHARSET` attribute. Storing them in `char` strips the attribute, so `mvwaddch` prints the raw letter (k/j/l/m) instead of line‑drawing. That is why the map shows random letters while the UI (which uses `box()` with proper `ACS_*`) looks correct.

## Required fix
- Use ncurses `ACS_*` values for line-drawing walls and store them in a `chtype` (not `char`) in `ui_render_map`.
- Keep the wall selection logic, but **do not** cast `ACS_*` to `char`.
- For `-` and `|`, assign them as `chtype` values. For corners, use `ACS_ULCORNER`, `ACS_URCORNER`, `ACS_LLCORNER`, `ACS_LRCORNER` so the `A_ALTCHARSET` bit is preserved.
- Continue to render unsupported junctions as a blank space `' '`.
- Keep the drawing order and window layout unchanged.

## Constraints
- Limit the change to rendering; do not rewrite map generation or UI layout.
- Build with `make` to ensure it compiles.

## Deliverables
- Code change(s) that make the map walls render with proper glyphs (no random letters).
- A short summary of what changed and why.
