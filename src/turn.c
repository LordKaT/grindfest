#include <stdlib.h>
#include <stdio.h>
#include "turn.h"

#define MAX_EVENTS 1024

static GameEvent heap[MAX_EVENTS];
static int heap_size = 0;
static long global_time = 0;
static long next_priority_id = 0;

void turn_init(void) {
    heap_size = 0;
    global_time = 0;
    next_priority_id = 0;
}

// Min-heap helpers
static void swap(int i, int j) {
    GameEvent temp = heap[i];
    heap[i] = heap[j];
    heap[j] = temp;
}

static int compare(GameEvent a, GameEvent b) {
    if (a.time != b.time) {
        return (a.time < b.time) ? -1 : 1;
    }
    // Stability tie-breaker
    return (a.priority_id < b.priority_id) ? -1 : 1;
}

static void sift_up(int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (compare(heap[idx], heap[parent]) < 0) {
            swap(idx, parent);
            idx = parent;
        } else {
            break;
        }
    }
}

static void sift_down(int idx) {
    int min_idx = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < heap_size && compare(heap[left], heap[min_idx]) < 0) {
        min_idx = left;
    }
    if (right < heap_size && compare(heap[right], heap[min_idx]) < 0) {
        min_idx = right;
    }

    if (idx != min_idx) {
        swap(idx, min_idx);
        sift_down(min_idx);
    }
}

void turn_add_event(long time, EntityID entity_id, EventType type) {
    if (heap_size >= MAX_EVENTS) return; // Error handling needed?

    GameEvent evt;
    evt.time = time;
    evt.entity_id = entity_id;
    evt.type = type;
    evt.priority_id = next_priority_id++;

    heap[heap_size] = evt;
    sift_up(heap_size);
    heap_size++;
}

GameEvent turn_pop_event(void) {
    if (heap_size == 0) {
        GameEvent empty = {0};
        return empty;
    }

    GameEvent root = heap[0];
    global_time = root.time; // Update global time to current event

    heap[0] = heap[heap_size - 1];
    heap_size--;
    sift_down(0);

    return root;
}

bool turn_queue_is_empty(void) {
    return heap_size == 0;
}

long turn_get_current_time(void) {
    return global_time;
}
