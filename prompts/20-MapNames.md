**Role:** You are a C Coding Agent.
**Task:** Implement Map Names, a UI Title Bar, and adjust the window layout to accommodate it.

**Context**
We want to display the name of the current zone (e.g., "Bastok Mines", "South Gustaberg") at the top of the map view. To do this without obscuring the map tiles, we need to increase the Map Window height by 1 line and decrease the Log Window height by 1 line (keeping the total terminal height at 24).

**1. Data Structures (`src/map.h`)**

* Add a name field to the `Map` struct:
```c
char name[64];

```

**2. Map Parsing (`src/map.c`)**

* Update `map_load_static`:
* Add parsing for the key `meta:name`.
* Example line: `meta:name=South Gustaberg`
* Read the rest of the line into `map->name`, stripping the newline.
* Default to "Unknown Area" if missing.
* Update `map_generate_dungeon`:
* Set `strcpy(map->name, "Procedural Dungeon");` (or similar).

**3. UI Layout Adjustments (`src/ui.c`)**
Refactor the height constants to make room for the title bar.

* **`MAP_VIEW_HEIGHT`**: Change from **16** to **17**.
* **`LOG_HEIGHT`**: Change from **7** to **6**.
* *Check:* 17 (Map) + 6 (Log) + 1 (Input) = 24. Correct.

**4. Map Rendering (`src/ui.c`)**
Update `ui_render_map` to draw the title and offset the map.

1. **Draw Title:** At window coordinate `(0, 1)` (or centered on line 0), print `"[ %s ]"`, `map->name`. Use `A_BOLD`.
2. **Offset Tiles:** When looping through `y` (0 to `MAP_HEIGHT`), draw the tiles at `window_y = y + 1`.
* *Example:* Map tile `(0,0)` draws at Window `(0,1)`. Map tile `(0,15)` draws at Window `(0,16)`.

3. **Horizontal Line (Optional):** You may draw a separator line or just rely on the box border if simpler, but the text header is the priority.

**5. Data Updates (`data/maps/*.map`)**

* Update `bastok.map`, `sandoria.map`, and `windurst.map` to include `meta:name=` definitions at the top.

**Deliverables**

1. `src/map.h`: Added `name` field.
2. `src/map.c`: Updated loader and generator to populate `name`.
3. `src/ui.c`: Updated height constants and rendering offset logic.
4. `data/maps/*.map`: Added name metadata.
