**Role:** You are a Python Tooling Expert.
**Task:** Build a robust, GUI-based Map Editor using `tkinter` for a C/ncurses roguelike engine.

**1. Core Philosophy & Scope**

* **Purpose:** This is a **Data Authoring Tool**, not a "Renderer Simulator". Visual fidelity (exact font matching) is secondary to data integrity.
* **Coordinate System:** `(0,0)` is **Top-Left**. +X is Right, +Y is Down. File rows map 1:1 to grid rows.
* **Destructive Editing:** The editor is **Authoritative**. It reads data into memory and re-serializes it completely on save. It does **not** preserve comments or unknown formatting from the source file.
* **Scope Constraints:**
* **Single Layer Only:** Only the "Terrain" layer exists for V1.
* **No Undo/Redo:** Explicitly out of scope for V1.
* **Performance:** Must handle maps up to **500x500** without input lag.

**2. Architecture (MVC)**

* **`MapModel`:** Stores the grid (list of lists of chars), width, height, and metadata (`name`, etc.).
* **`MapView`:** Handles `tkinter` widgets and canvas rendering.
* **`EditorController`:** Bridges UI events to Model updates.

**3. Data Model Constraints**

* **Grid Size:** Default to **54x16** (matching the game's Viewport size), but support resizing to arbitrary dimensions (up to 500x500).
* **Valid Glyphs:**
* Any **single Unicode code point** is allowed.
* *Constraint:* Treat all glyphs as logical `width=1`.
* *Void State:* The character `' '` (Space) represents Void/Empty/Erase. There is no distinction between "Unpainted" and "Space".

**4. UI Layout**

* **Menu Bar:** Standard File menu (New, Open, Save, Save As, Exit). "New" should prompt for Width/Height.
* **Toolbar (Top):**
* **Active Glyph Input:** A `tk.Entry`.
* *Validation:* Enforce length = 1. If a user pastes a string, take the first code point only.

* **Zoom Controls:** `[+]` / `[-]` buttons.
* *Behavior:* Changes the Canvas font size. **View-only**; does not affect saved data. Not persistent across sessions.

* **Palette (Left):**
* Buttons for common tiles: `#` (Wall), `.` (Floor), `~` (Water), `+` (Door), `>` (Stairs).
* **Void Button:** Labeled `[Space]`, inserts `' '`.
* *Interaction:* Clicking a button updates the "Active Glyph Input" entry.

* **Canvas (Center):**
* Scrollable `tk.Canvas`.
* **Rendering:** Use `create_text` for grid cells.
* **Optimization:** Use `itemconfig` to update existing text IDs. Do not delete/recreate tags every frame.

**5. Mouse Interaction**

* **Painting (Left Click/Drag):**
* Sets cell to **Active Glyph**.
* **Continuous:** Dragging paints continuously.
* **Idempotent:** Do not trigger a model update/redraw if the cell already contains the target glyph (prevents lag).

* **Erasing (Right Click/Drag):**
* Sets cell to `' '` (Space). Same continuous/idempotent rules apply.

**6. File I/O (Strict Spec)**

* **Format:**
```text
meta:width=54
meta:height=16
meta:name=My Map
layer:terrain
################
#..............#

```

* **Loading (Permissive):**
* Parse `meta:width` / `meta:height`. Resize model.
* Store other `meta:` keys in a dictionary.
* **Discard** lines starting with `%` (comments) or unknown keys.
* Read `layer:terrain` rows.
* *Error Handling:* If rows are too short, pad with spaces. If too long, truncate. Warn the user via console/dialog but load the file.


* **Saving (Strict):**
* Write header based on *current* Model dimensions.
* Write stored metadata.
* Write `layer:terrain`.
* Write grid rows exactly as stored.

**7. Deliverables**

* A single Python script `map_editor.py` containing the `MapModel`, `MapView`, and `EditorController` classes.
* The script must be runnable immediately (`python map_editor.py`).

**Implementation Note for Agent:**
Ensure the Canvas scroll region updates correctly when the map size changes or zoom level changes.