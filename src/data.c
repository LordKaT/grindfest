#include <stdio.h>
#include <stdlib.h>
#include "data.h"
#include "ui.h"

void data_init_loaders(void) {
    // Stub
}

void data_load_jobs(const char* filename) {
    // Stub implementation
    FILE* f = fopen(filename, "r");
    if (!f) {
        ui_log("Error: Could not open %s", filename);
        return;
    }
    // Parse logic here
    fclose(f);
    ui_log("Loaded Jobs from %s", filename);
}

void data_load_monsters(const char* filename) {
    // Stub implementation
    FILE* f = fopen(filename, "r");
    if (!f) {
        ui_log("Error: Could not open %s", filename);
        return;
    }
    // Parse logic here
    fclose(f);
    ui_log("Loaded Monsters from %s", filename);
}
