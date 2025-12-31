#include <ncurses.h>
#include <string.h>
#include "input.h"
#include "combat.h"
#include "game.h"
#include "ui.h"

InputResult input_handle_key(int key) {
    InputResult res = {0};
    res.type = INPUT_ACTION_NONE;

    switch(key) {
        case '8':
            res.type = INPUT_ACTION_MOVE_UP;
            break;
        case '2':
            res.type = INPUT_ACTION_MOVE_DOWN;
            break;
        case '4':
            res.type = INPUT_ACTION_MOVE_LEFT;
            break;
        case '6':
            res.type = INPUT_ACTION_MOVE_RIGHT;
            break;
        case '7':
            res.type = INPUT_ACTION_MOVE_UP_LEFT;
            break;
        case '9':
            res.type = INPUT_ACTION_MOVE_UP_RIGHT;
            break;
        case '1':
            res.type = INPUT_ACTION_MOVE_DOWN_LEFT;
            break;
        case '3':
            res.type = INPUT_ACTION_MOVE_DOWN_RIGHT;
            break;
        case 27: // ESC
            res.type = INPUT_ACTION_QUIT;
            break;
        case '/':
            res.type = INPUT_ACTION_COMMAND;
            break;
        case '5':
            res.type = INPUT_ACTION_WAIT;
            break;
        case KEY_F(1):
            res.type = INPUT_ACTION_VIEW_NORMAL;
            break;
        case KEY_F(2):
            res.type = INPUT_ACTION_VIEW_SMELL;
            break;
        case KEY_F(3):
            res.type = INPUT_ACTION_VIEW_SOUND;
            break;
        default:
            break;
    }
    return res;
}

void input_parse_command(const char* cmd_str, Entity* player, EntityID* target_out) {
    char cmd[64];
    char arg1[64];
    (void)target_out; // Unused for now

    int count = sscanf(cmd_str, "%s %s", cmd, arg1);
    if (count < 1) return;
    
    if (strcmp(cmd, "/attack") == 0) {
        // Find a target if not provided
        EntityID tid = -1;
        
        // Very simple logic: If <t> is passed, use player->target_id
        // If just /attack, pick closest.
        
        // Find first active entity that is NOT the player
        for (int i=0; i<g_game.entity_count; i++) {
            if (g_game.entities[i].id != player->id && g_game.entities[i].is_active) {
                tid = g_game.entities[i].id;
                break; // Found one
            }
        }
        
        if (tid != -1) {
             combat_engage(player, tid);
        } else {
            ui_log("No valid target found.");
        }
        
    } else if (strcmp(cmd, "/check") == 0) {
        ui_log("Check command not impl.");
    } else {
        ui_log("Unknown command: %s", cmd);
    }
}
