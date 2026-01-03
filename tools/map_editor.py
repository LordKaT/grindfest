#!/usr/bin/env python3
import tkinter as tk
from tkinter import filedialog, messagebox, simpledialog, font as tkfont
import sys
import os
from typing import List, Dict, Tuple, Optional

# Constants
DEFAULT_WIDTH = 54
DEFAULT_HEIGHT = 16
DEFAULT_FONT_SIZE = 12
CELL_PADDING = 2

class MapModel:
    def __init__(self, width: int = DEFAULT_WIDTH, height: int = DEFAULT_HEIGHT, name: str = "Untitled"):
        self.width = width
        self.height = height
        self.name = name
        self.metadata: Dict[str, str] = {
            "width": str(width),
            "height": str(height),
            "name": name
        }
        self.grid: List[List[str]] = [[' ' for _ in range(width)] for _ in range(height)]

    def resize(self, width: int, height: int):
        new_grid = [[' ' for _ in range(width)] for _ in range(height)]
        for y in range(min(self.height, height)):
            for x in range(min(self.width, width)):
                new_grid[y][x] = self.grid[y][x]
        self.width = width
        self.height = height
        self.metadata["width"] = str(width)
        self.metadata["height"] = str(height)
        self.grid = new_grid

    def set_cell(self, x: int, y: int, char: str):
        if 0 <= x < self.width and 0 <= y < self.height:
            self.grid[y][x] = char

    def get_cell(self, x: int, y: int) -> str:
        if 0 <= x < self.width and 0 <= y < self.height:
            return self.grid[y][x]
        return ' '

    def load_from_string(self, content: str):
        lines = content.splitlines()
        meta = {}
        layer_terrain_found = False
        terrain_lines = []
        
        # Pass 1: Parse headers
        idx = 0
        while idx < len(lines):
            line = lines[idx]
            if line.startswith("meta:"):
                try:
                    key, val = line.split(":", 2)[1].split("=", 1)
                    meta[key.strip()] = val.strip()
                except ValueError:
                    pass # Invalid meta line
            elif line.startswith("layer:terrain"):
                layer_terrain_found = True
                idx += 1
                break
            elif line.startswith("%"):
                pass # Comment
            idx += 1
            
        # Parse Dimensions
        try:
            w = int(meta.get("width", DEFAULT_WIDTH))
            h = int(meta.get("height", DEFAULT_HEIGHT))
        except ValueError:
            w, h = DEFAULT_WIDTH, DEFAULT_HEIGHT
            
        self.resize(w, h)
        self.name = meta.get("name", "Untitled")
        self.metadata = meta

        # Pass 2: Read Grid
        row = 0
        while idx < len(lines) and row < self.height:
            line_content = lines[idx]
            
            # Truncate
            data = line_content[:self.width]
            # Pad
            data = data.ljust(self.width, ' ')
            
            for col, char in enumerate(data):
                self.set_cell(col, row, char)
            
            row += 1
            idx += 1

    def to_string(self) -> str:
        # Update metadata before saving
        self.metadata["width"] = str(self.width)
        self.metadata["height"] = str(self.height)
        self.metadata["name"] = self.name
        
        output = []
        # Meta
        for k, v in self.metadata.items():
            output.append(f"meta:{k}={v}")
        
        # Layer
        output.append("layer:terrain")
        
        # Grid
        for row in self.grid:
            output.append("".join(row))
            
        return "\n".join(output) + "\n"

class MapView:
    def __init__(self, master: tk.Tk, controller):
        self.master = master
        self.controller = controller
        self.font_size = DEFAULT_FONT_SIZE
        self.font = ("Courier New", self.font_size, "bold")
        self.cell_width = 10 # Will be calculated
        self.cell_height = 18 # Will be calculated
        self.canvas_ids = {} # (x,y) -> item_id
        
        self.create_menu()
        self.create_toolbar()
        self.create_layout()

    def create_menu(self):
        menubar = tk.Menu(self.master)
        filemenu = tk.Menu(menubar, tearoff=0)
        filemenu.add_command(label="New", command=self.controller.new_map)
        filemenu.add_command(label="Open", command=self.controller.open_map)
        filemenu.add_command(label="Save", command=self.controller.save_map)
        filemenu.add_command(label="Save As...", command=self.controller.save_as_map)
        filemenu.add_separator()
        filemenu.add_command(label="Exit", command=self.master.quit)
        menubar.add_cascade(label="File", menu=filemenu)
        self.master.config(menu=menubar)

    def create_toolbar(self):
        toolbar = tk.Frame(self.master, bd=1, relief=tk.RAISED)
        toolbar.pack(side=tk.TOP, fill=tk.X)
        
        tk.Label(toolbar, text="Glyph:").pack(side=tk.LEFT, padx=2)
        
        self.glyph_var = tk.StringVar(value="#")
        self.glyph_var.trace("w", lambda *args: self.validate_glyph())
        self.glyph_entry = tk.Entry(toolbar, textvariable=self.glyph_var, width=3, font=("Courier", 12))
        self.glyph_entry.pack(side=tk.LEFT, padx=2)
        
        tk.Frame(toolbar, width=20).pack(side=tk.LEFT) # Spacer
        
        tk.Button(toolbar, text="Zoom In (+)", command=self.zoom_in).pack(side=tk.LEFT)
        tk.Button(toolbar, text="Zoom Out (-)", command=self.zoom_out).pack(side=tk.LEFT)
        
        self.status_var = tk.StringVar()
        tk.Label(toolbar, textvariable=self.status_var).pack(side=tk.RIGHT, padx=5)

    def validate_glyph(self):
        val = self.glyph_var.get()
        if len(val) > 1:
            self.glyph_var.set(val[0])
        elif len(val) == 0:
            pass # Keep empty

    def create_layout(self):
        main_frame = tk.Frame(self.master)
        main_frame.pack(fill=tk.BOTH, expand=True)

        # Palette
        palette_frame = tk.Frame(main_frame, width=100, bg="#e0e0e0")
        palette_frame.pack(side=tk.LEFT, fill=tk.Y, padx=0, pady=0)
        
        common_glyphs = [
            ("#", "Wall"),
            (".", "Floor"),
            ("~", "Water"),
            ("+", "Door"),
            (">", "Stairs")
        ]
        
        for char, lbl in common_glyphs:
            btn = tk.Button(palette_frame, text=f"{char} {lbl}", font=("Courier", 10),
                            command=lambda c=char: self.controller.select_glyph(c))
            btn.pack(fill=tk.X, padx=2, pady=1)
            
        tk.Button(palette_frame, text="[Space] Void", font=("Courier", 10),
                  command=lambda: self.controller.select_glyph(' ')).pack(fill=tk.X, padx=2, pady=5)

        # Canvas container with scrollbars
        canvas_frame = tk.Frame(main_frame)
        canvas_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        self.h_scroll = tk.Scrollbar(canvas_frame, orient=tk.HORIZONTAL)
        self.v_scroll = tk.Scrollbar(canvas_frame, orient=tk.VERTICAL)
        
        self.canvas = tk.Canvas(canvas_frame, bg="black", 
                                xscrollcommand=self.h_scroll.set, 
                                yscrollcommand=self.v_scroll.set)
        
        self.h_scroll.config(command=self.canvas.xview)
        self.v_scroll.config(command=self.canvas.yview)
        
        self.h_scroll.pack(side=tk.BOTTOM, fill=tk.X)
        self.v_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        # Bindings
        self.canvas.bind("<Button-1>", self.controller.on_canvas_click)
        self.canvas.bind("<B1-Motion>", self.controller.on_canvas_drag)
        self.canvas.bind("<Button-3>", self.controller.on_canvas_rclick)
        self.canvas.bind("<B3-Motion>", self.controller.on_canvas_rdrag)

    def calculate_metrics(self):
        f = tkfont.Font(font=self.font)
        self.cell_width = f.measure("W")
        self.cell_height = f.metrics("linespace")

    def init_grid_rendering(self, width, height):
        self.canvas.delete("all")
        self.canvas_ids = {}
        self.calculate_metrics()
        
        total_w = width * self.cell_width
        total_h = height * self.cell_height
        self.canvas.config(scrollregion=(0, 0, total_w, total_h))
        
        for y in range(height):
            for x in range(width):
                 cx = x * self.cell_width + self.cell_width // 2
                 cy = y * self.cell_height + self.cell_height // 2
                 item_id = self.canvas.create_text(
                     cx, cy, 
                     text=' ', 
                     font=self.font, 
                     fill="white",
                     anchor="center"
                 )
                 self.canvas_ids[(x, y)] = item_id

    def update_cell(self, x, y, char):
        if (x, y) in self.canvas_ids:
            color = "white"
            if char == '#': color = "#888888"
            elif char == "~": color = "#4444ff"
            elif char == "+": color = "yellow"
            elif char == ">": color = "red"
            elif char == ".": color = "#cccccc"
            
            self.canvas.itemconfigure(self.canvas_ids[(x, y)], text=char, fill=color)

    def zoom_in(self):
        self.font_size += 2
        self.refresh_font()

    def zoom_out(self):
        if self.font_size > 4:
            self.font_size -= 2
        self.refresh_font()
        
    def refresh_font(self):
        self.font = ("Courier New", self.font_size, "bold")
        self.controller.refresh_view_full()

    def screen_to_grid(self, sx, sy):
        x = int(self.canvas.canvasx(sx) // self.cell_width)
        y = int(self.canvas.canvasy(sy) // self.cell_height)
        return x, y

class EditorController:
    def __init__(self, root):
        self.root = root
        self.root.title("Grindfest Map Editor")
        self.root.geometry("800x600")
        
        self.model = MapModel()
        self.view = MapView(root, self)
        
        self.current_filename = None
        
        self.refresh_view_full()

    def refresh_view_full(self):
        self.view.init_grid_rendering(self.model.width, self.model.height)
        for y in range(self.model.height):
            for x in range(self.model.width):
                self.view.update_cell(x, y, self.model.get_cell(x, y))
        self.update_status()

    def update_status(self):
        name = self.model.name
        dim = f"{self.model.width}x{self.model.height}"
        self.view.status_var.set(f"Map: {name} [{dim}]")

    def new_map(self):
        w = simpledialog.askinteger("New Map", "Width:", initialvalue=DEFAULT_WIDTH, minvalue=1, maxvalue=500)
        h = simpledialog.askinteger("New Map", "Height:", initialvalue=DEFAULT_HEIGHT, minvalue=1, maxvalue=500)
        if w and h:
             self.model = MapModel(w, h, "Untitled")
             self.current_filename = None
             self.refresh_view_full()

    def open_map(self):
        path = filedialog.askopenfilename(filetypes=[("Map Files", "*.map"), ("All Files", "*.*")])
        if path:
            try:
                with open(path, 'r') as f:
                    content = f.read()
                self.model.load_from_string(content)
                self.current_filename = path
                self.refresh_view_full()
            except Exception as e:
                messagebox.showerror("Error", f"Failed to load map: {e}")

    def save_map(self):
        if self.current_filename:
            self.save_to_file(self.current_filename)
        else:
            self.save_as_map()

    def save_to_file(self, path):
        try:
            with open(path, 'w') as f:
                f.write(self.model.to_string())
            self.current_filename = path
            messagebox.showinfo("Success", f"Map saved to {path}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to save map: {e}")

    def save_as_map(self):
        path = filedialog.asksaveasfilename(defaultextension=".map", filetypes=[("Map Files", "*.map")])
        if path:
            self.save_to_file(path)

    def select_glyph(self, char):
        self.view.glyph_var.set(char)

    def get_active_glyph(self):
        val = self.view.glyph_var.get()
        return val[0] if val else ' '

    def on_canvas_click(self, event):
        self.paint(event.x, event.y)

    def on_canvas_drag(self, event):
        self.paint(event.x, event.y)

    def on_canvas_rclick(self, event):
        self.erase(event.x, event.y)
        
    def on_canvas_rdrag(self, event):
        self.erase(event.x, event.y)

    def paint(self, screen_x, screen_y):
        x, y = self.view.screen_to_grid(screen_x, screen_y)
        glyph = self.get_active_glyph()
        current = self.model.get_cell(x, y)
        
        if current != glyph:
            self.model.set_cell(x, y, glyph)
            self.view.update_cell(x, y, glyph)

    def erase(self, screen_x, screen_y):
        x, y = self.view.screen_to_grid(screen_x, screen_y)
        glyph = ' '
        current = self.model.get_cell(x, y)
        if current != glyph:
            self.model.set_cell(x, y, glyph)
            self.view.update_cell(x, y, glyph)

if __name__ == "__main__":
    root = tk.Tk()
    app = EditorController(root)
    root.mainloop()
