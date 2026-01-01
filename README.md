# Grindfest

A minimal roguelike scaffold in pure C (ncurses), featuring an FFXI-style "Engagement" combat system.

Yes, this project is mostly vibe coded. No, I don't care. If you don't like it, go somewhere else. It's my project not yours.

## Build Instructions

```bash
make
./bin/grindfest
```

## Key Features

*   **Turn System**: A priority queue scheduler handles time.
*   **Engagement Combat**:
    *   Unlike traditional roguelikes ("bump to attack"), moving into an enemy does NOT attack.
    *   You must engage an enemy using `/attack` (or a macro).
    *   Once engaged, **Auto-Attacks** happen automatically in the background based on a timer (`Event Priority Queue`).
    *   You are free to move or type commands *while* your character trades blows with the enemy.
*   **Macro System**: The bottom line accepts slash commands like `/attack` and `/ws`.

## Project Structure

*   `src/`: Source code.
    *   `main.c`: Entry point.
    *   `game.c`: State machine and main loop.
    *   `turn.c`: Min-heap priority queue scheduler.
    *   `combat.c`: Engagement and auto-attack logic.
    *   `entity.h`: Core data structures (Entity, Stats, Jobs).
    *   `input.c`: Command parser.
    *   `ui.c`: Ncurses rendering.
*   `data/`: Data files for Jobs and Monsters.

## Engagement Logic Explanation

The game uses a global Priority Queue for time management. Actions have a cost in "ticks".

1.  **Movement**: When the player acts (moves), it schedules the next `EVENT_MOVE` logic N ticks in the future.
2.  **Combat**: When Engaged, a separate chain of `EVENT_ATTACK_READY` events is spawned.
    *   These events occur automatically based on `WeaponDelay`.
    *   The `game.c` loop processes these "Attack Ticks" without blocking for user input.
    *   Code Reference: `src/combat.c` schedules the initial attack, and `src/game.c` reschedules subsequent attacks.

This split allows the "real-time" feel of an MMO auto-attack loop within a deterministic turn-based structure.
