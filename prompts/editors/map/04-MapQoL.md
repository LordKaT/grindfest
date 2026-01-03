**Role:** You are a Python Tooling Expert.
**Task:** Add Quality-of-Life features (Hover Cursor & Status Bar) to the Map Editor.

**Context**
The Map Editor functions well but lacks visual feedback. Users cannot see exactly which cell they are targeting until they click, and they don't know the specific coordinates of the grid. We need to add "Passive Mouse Tracking" to solve this.

**1. UI Updates (`MapView` in `tools/map_editor.py`)**

* **Status Bar:**
* Add a new `tk.Label` widget at the very bottom of the main window (`side=tk.BOTTOM, fill=tk.X`).
* Style: `relief=tk.SUNKEN`, `anchor=tk.E` (East/Right aligned).
* Initialize text to `"Pos: (-, -)"`.

* **Ghost Cursor (The Hover Preview):**
* We need a transient visual indicator on the Canvas.
* **Implementation:** In `init_grid_rendering`, create a **new** text item on the canvas with a unique tag (e.g., `"cursor_preview"`).
* **Style:** Use a distinct color (e.g., "gold" or "orange") and `A_BOLD` font to differentiate it from placed tiles.
* **Helper Method:** Add `update_cursor_preview(self, grid_x, grid_y, char)`:
* Calculate canvas `cx, cy` for the given grid coordinates.
* Use `self.canvas.coords` to move the `"cursor_preview"` item to that location.
* Use `self.canvas.itemconfig` to update its text to `char`.
* *Note:* Ensure this item is raised above grid cells (`self.canvas.tag_raise("cursor_preview")`).

**2. Controller Logic (`EditorController` in `tools/map_editor.py`)**

* **Event Binding:**
* Bind `<Motion>` events on the Canvas to a new handler `on_mouse_move`.
* Bind `<Leave>` events on the Canvas to a handler `on_mouse_leave`.

* **Handlers:**
* `on_mouse_move(event)`:
1. Convert screen `event.x, event.y` to grid `gx, gy`.
2. **Status Bar:** Update the status bar text to `f"Pos: ({gx}, {gy})"`.
3. **Ghost Cursor:**
* Get the active glyph (if Pencil tool) or current tool icon.
* Call `view.update_cursor_preview(gx, gy, glyph)`.
* *Constraint:* If `active_tool == "bucket"`, you may display the bucket glyph (or just the paint glyph) as the preview.

* `on_mouse_leave(event)`:
* Hide the ghost cursor (move it off-screen or set text to empty).
* Reset status bar to `"Pos: (-, -)"`.

**3. Interaction Polish**

* **Click vs. Hover:**
* The Ghost Cursor must **not** modify the actual `MapModel` data. It is purely visual.
* Left-click still triggers `paint` (which commits to Model).
* Right-click still triggers `erase`.

* **Erase Preview:**
* If the user is holding Right-click (erasing), the ghost cursor logic generally doesn't run (as `B3-Motion` takes over). This is acceptable.
* *Optional:* If you want to be fancy, right-click dragging could show a "Red X" or empty space ghost, but standard behavior is fine.

**Deliverables**

* Updated `tools/map_editor.py` including the new Status Bar widget, Canvas Ghost item, and Mouse Motion handlers.