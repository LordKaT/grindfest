**Role:** You are a Python Tooling Expert.
**Task:** Add a Flood Fill (Bucket) tool and Tool Selection logic to the Map Editor.

**Context**
The Map Editor currently operates only in "Pencil Mode" (clicking paints a single cell). We need to add a "Bucket Fill" mode to fill connected areas with the active glyph. This requires adding a concept of "Active Tool" to the controller and UI.

**1. Tool Logic (`EditorController` in `tools/map_editor.py`)**

* **State:** Add `self.active_tool` to the controller. Valid values: `"pencil"`, `"bucket"`. Default is `"pencil"`.
* **Methods:**
* `set_tool(tool_name)`: Updates `active_tool`.
* `flood_fill(start_x, start_y, fill_glyph)`:
* Implement a standard **Breadth-First Search (BFS)** (Queue-based) flood fill. **Do NOT use recursion** (to avoid stack overflow on large maps).
* *Logic:*
1. Get `target_glyph` (the glyph currently at `start_x, start_y`).
2. If `target_glyph == fill_glyph`, return (no-op).
3. Queue `(start_x, start_y)`.
4. While queue not empty:
* Pop `(x, y)`.
* If `model.get_cell(x, y) != target_glyph`: continue.
* Set cell `(x, y)` to `fill_glyph`.
* Update View (`view.update_cell`).
* Check 4 neighbors (N/S/E/W). If neighbor is within bounds AND matches `target_glyph`, add to queue.

* **Input Handling:** Update `on_canvas_click`:
* If `active_tool == "pencil"`: Call `self.paint(event.x, event.y)`.
* If `active_tool == "bucket"`:
* Convert screen `x,y` to grid `gx, gy`.
* Call `self.flood_fill(gx, gy, self.get_active_glyph())`.

**2. UI Updates (`MapView` in `tools/map_editor.py`)**

* **Tool Palette:**
* Add a new `LabelFrame` or section in the Left Sidebar (at the very top, above the glyph palette) labeled "Tools".
* **Buttons:**
* `[P]encil`: Sets tool to `"pencil"`.
* `[F]ill`: Sets tool to `"bucket"`.

* *Visual Feedback:* Implement a way to show which tool is active (e.g., change the active button's background to "gray" or `relief` to `SUNKEN`, while the other is `RAISED`).

* **Hotkeys:**
* Bind key `<p>` to set Pencil tool.
* Bind key `<f>` to set Bucket tool.
* Bind these on the `root` window.

**3. Interaction Rules**

* **Dragging:**
* Pencil: Supports drag-to-paint (existing behavior).
* Bucket: **Disable dragging.** Flood fill should only trigger on `ButtonPress` (click), never on `B1-Motion` (drag).
* Update `on_canvas_drag` to simply `return` if `active_tool == "bucket"`.

**Deliverables**

* Updated `tools/map_editor.py` with the new Flood Fill algorithm, Tool Selection UI, and Hotkey bindings.
