### Prompt for Coding Agent

Role: You are a C Coding Agent.
Task: Resolve ambiguity in the Turn/Event loop and implement the "Wait" action.

Context (Roguelike Time Model)
1) Strict turn-based: the game does NOT advance in real-time ticks/frames. It waits indefinitely for player input.
2) Input drives time: time advances only when the player moves, acts, or waits.
3) Auto-attacks are scheduled events in the same turn-cost timeline. They do NOT preempt input; they interleave based on turn costs.
4) All references to “time” in this system are turn-cost units, not ticks or frames. Avoid language that implies real-time.

1) Code Cleanup (src/game.c)
- Locate the large comment block starting with `// PROBLEM: "Movement and Attacking are separate event chains"`.
- Replace it with:
```c
// Turn Logic (turn-cost units, not real-time):
// 1. Pop the next event by scheduled time.
// 2. If it is the player's Move event, block for input.
// 3. Input determines action cost and schedules the next Move event.
// 4. Any intermediate events (like auto-attacks) scheduled between now and the
//    next Move event will be popped and processed in order before input resumes.
```

2) Implement "Wait" (Keypad 5)
- src/input.h: Add `INPUT_ACTION_WAIT` to `InputAction`.
- src/input.c: Map `'5'` (numpad 5 / digit 5) to `INPUT_ACTION_WAIT`.
  - Do NOT add `.` or any non-numpad movement/utility keys.
- src/game.c: In the input handling loop in `update_dungeon`, handle `INPUT_ACTION_WAIT`:
  - Log: "You wait."
  - Set `turn_taken = true`.
  - Schedule next `EVENT_MOVE` at `evt.time + 100` (standard action cost).

3) Auto-Attack Tie-Break Rule
- Clarify how same-time events are ordered.
- In `src/turn.c`, when events have the same `time`, the queue uses `priority_id` insertion order.
- In `src/game.c`, add a short comment noting: if a player move event and an auto-attack have the same scheduled time, the one enqueued earlier runs first (current behavior). Do NOT change scheduler logic.

4) UI Label Clarity (Optional but preferred)
- If the UI displays "Time" anywhere, add a brief comment or rename it to "Turn" to reinforce that it is turn-cost time, not wall-clock time.

Deliverables
1) `src/game.c`: Updated comments and Wait logic.
2) `src/input.c` / `src/input.h`: Added Wait action for key `'5'` only.
3) `src/game.c` or `src/turn.c`: Note the same-time tie-break behavior (comment only).
