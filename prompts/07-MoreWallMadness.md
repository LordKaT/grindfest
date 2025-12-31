You are an external coding agent. Refactor wall rendering in `src/ui.c` to use a direction mask + lookup table (no complex if/else), and switch to wide‑char ncurses output so line drawing is stable.

## Current state (audit)
- `ui_render_map` computes a mask inline and switches through many cases.
- It mixes floor/void adjacency and uses debug glyphs (`'0'`) in some cases.
- The logic is hard to reason about and produces incorrect corners/tees.

## Required approach
Implement the following structure:

1) Define direction mask bits:
```c
enum { DIR_N = 1, DIR_E = 2, DIR_S = 4, DIR_W = 8 };
```

2) Define a 16‑entry wall glyph table (use wide‑char compatible ncurses glyphs):
```c
static const wchar_t WALL_GLYPH[16] = {
    L' ',           // 0  - .... isolated
    WACS_VLINE,     // 1  - N...
    WACS_HLINE,     // 2  - .E..
    WACS_LLCORNER,  // 3  - NE..
    WACS_VLINE,     // 4  - ..S.
    WACS_VLINE,     // 5  - N.S.
    WACS_ULCORNER,  // 6  - .ES.
    WACS_RTEE,      // 7  - NES.
    WACS_HLINE,     // 8  - ...W
    WACS_LRCORNER,  // 9  - N..W
    WACS_VLINE,     // 10 - .E.W
    WACS_UTEE,      // 11 - NE.W
    WACS_URCORNER,  // 12 - ..SW
    WACS_LTEE,      // 13 - N.SW
    WACS_BTEE,      // 14 - .ESW
    WACS_PLUS       // 15 - NESW
};
```
If `WACS_*` is unavailable in your ncurses build, fall back to `ACS_*` but still keep the table approach.

3) Add `connects_to_wall(tile)`:
- Return true only for wall tiles that should connect to other walls.
- Do **not** connect through floors, void, or out‑of‑bounds.

4) Add `wall_mask_at(x, y)`:
```c
int wall_mask_at(int x, int y) {
    int m = 0;
    if (connects_to_wall(tile_at(x, y-1))) m |= DIR_N;
    if (connects_to_wall(tile_at(x+1, y))) m |= DIR_E;
    if (connects_to_wall(tile_at(x, y+1))) m |= DIR_S;
    if (connects_to_wall(tile_at(x-1, y))) m |= DIR_W;
    return m;
}
```
Provide a safe `tile_at` helper that handles bounds (treat out‑of‑bounds as non‑wall).

5) Add `wall_glyph_at(x, y)`:
```c
wchar_t wall_glyph_at(int x, int y) {
    return WALL_GLYPH[wall_mask_at(x, y)];
}
```

6) Replace the current wall switch in `ui_render_map` with the table lookup:
- For floor tiles, keep `.`.
- For wall tiles, call `wall_glyph_at` and draw the resulting glyph.
- Remove any debug `0` glyphs.

## Wide‑char output requirements
- Use ncursesw: include `<locale.h>` and call `setlocale(LC_ALL, "")` during init.
- Use wide‑char drawing functions (`mvwadd_wch` or `mvwaddnwstr` with a `cchar_t`) so `WACS_*` renders correctly.
- Update the Makefile to link `-lncursesw` if necessary.

## Scope
- Primary changes in `src/ui.c` (and `src/ui.h` if helpers need declarations).
- Minor Makefile changes allowed for ncursesw.
- Do not alter map generation logic.

## Deliverables
- Code changes implementing the mask‑table wall drawing + wide‑char output.
- Short summary of what changed and why it fixes the wall rendering issues.
