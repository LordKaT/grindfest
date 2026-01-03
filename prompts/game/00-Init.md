You are a coding agent. Build a minimal-but-solid roguelike scaffold in pure C (NOT C++), targeting an 80x24 terminal. The goal is to establish a carefully organized, extensible codebase that a human can easily edit and extend later.

**Non-negotiables**

* **Language:** C (C99 or C11). No C++.
* **UI:** Terminal-based, 80 columns x 24 rows. Use ncurses (or PDCurses on Windows if you add notes), but default to ncurses on Linux.
* **Project flow:** Start Game -> Character Creator -> Dungeon.
* **Core Loop Philosophy:** This is **NOT** a standard "bump-to-attack" roguelike. It is a "menu/macro-based" grinder inspired by Final Fantasy XI. The code structure must reflect this state machine (Idle -> Engaged -> Auto-Attacking -> Weapon Skill).

**UI Layout (80x24)**

* **Total:** 80 cols x 24 rows
* **Right panel (Multi-use):** 26 cols (x = 54..79). Modes: Stats, Inventory, Equipment, Logs.
* **Left area (Dungeon):** 54 cols (x = 0..53), height 16 rows (y = 0..15).
* **Bottom-left (Log + Input):** Width 54 cols. Log lines (y = 16..22). Input line (y = 23).
* **Requirement:** The Right Panel must be a switchable "mode" system so new panes (e.g., "Macro Palette") can be added later.

**Gameplay Scope & Mechanics**

1. **Character & Stats (FFXI Data Structure)**
* **Attributes:** STR, DEX, VIT, AGI, INT, MND, CHR.
* **Resources:** HP/MaxHP, **TP (Tactical Points, 0-3000)**, MP/MaxMP.
* **Job System:** Struct must support a `MainJob` and `SubJob` ID (even if SubJob logic is stubbed for now).
* **Creator:** Simple name input + Job selection from a file-loaded list.

2. **Dungeon & Respawn System (The "Camp" Mechanic)**
* **Generation:** BSP rooms + Drunken Walk tunnels (40x40 min size).
* **Spawning:**
* Do not just scatter mobs once.
* Implement a **Respawn System**. When an entity dies, it leaves a "tombstone" (invisible marker) that triggers a respawn of that mob type after N turns.
* This allows the player to "camp" a room and grind respawns.

3. **Combat: The Engagement System (Crucial)**
* **No Bump-to-Kill:** Walking into an enemy does **not** deal damage. It toggles **Engagement Mode**.
* **Auto-Attack Loop:**
* When Engaged, the player and target exchange basic attacks automatically based on a `WeaponDelay` stat (integrated into the turn scheduler).
* Successful auto-attacks generate TP.

* **Player Freedom:** While Engaged (auto-attacking), the player can still move (positioning) or type commands (macros).
* **Disengage:** Moving too far from the target breaks Engagement.

4. **Input: The "Macro" Command Line**
* The bottom input line is the primary interface for skills, not just a debug tool.
* Implement a parser that supports `(Action) "Name" (Target)`.
* **Targeting Placeholders:** The parser must resolve `<t>` (current target) and `<me>` (self).
* **Example Commands to scaffold:**
* `/attack <t>` (Manual engage)
* `/check <t>` (Inspect enemy strength)
* `/ws "Heavy Swing" <t>` (Uses TP to deal damage—stub this logic)

**Turn System (Agility + Delay)**
Implement a priority queue scheduler (min-heap or sorted list of events).

* Entities have `next_action_time`.
* **Movement Cost:** Based on Agility/Movement Speed.
* **Attack Cost:** Based on Weapon Delay.
* *Note:* Movement and Attacking are separate event chains. A player can move *between* their own auto-attack swings.

**Field of View (FoV)**

* Recursive shadowcasting.
* **Ghosting:** Enemies seen previously but currently out of FoV are drawn as static "ghosts" at their last known location (shaded/dim).

**Data-Driven Content**

* **Format:** External text files (CSV or simple key=value).
* **Files:** `jobs.txt`, `monsters.txt`, `weapons.txt`.
* **Error Handling:** Fail gracefully with a message if data is missing.

**Project Structure (Required)**
Deliver a buildable repository layout:

* `/src`
* `main.c`
* `game.c/.h` (State machine: Start -> Creator -> DungeonLoop)
* `ui.c/.h` (Panels, log, input line rendering)
* `input.c/.h` (Key handling + **Command Parser**)
* `combat.c/.h` (**Engagement logic**, Auto-attack calcs, TP gain)
* `entity.c/.h` (Structs with Main/Sub jobs, TP, Stats)
* `turn.c/.h` (Priority queue scheduler)
* `map.c` / `gen.c` / `fov.c`
* `data.c/.h` (File loaders)


* `/data` (Sample files for Jobs/Mobs)
* `Makefile`
* `README.md`

**Quality Bar**

* **Pure C:** No C++.
* **Memory:** Clean up resources on exit. No leaks in the main loop.
* **Extensibility:** Code must be commented specifically where new "Weapon Skills" or "Spells" should be hooked in.

**Deliverables**

1. All source code files.
2. Makefile.
3. Sample data files (Jobs, Monsters).
4. README explaining how the **Engagement/Auto-attack loop** works in the turn-based structure.
