#ifndef INPUT_H
#define INPUT_H

#include "entity.h"

typedef enum {
    INPUT_ACTION_NONE,
    INPUT_ACTION_CONFIRM, // Enter
    INPUT_ACTION_CANCEL, // ESC or Quit
    INPUT_ACTION_MENU,   // 'm'
    INPUT_ACTION_MOVE_UP,
    INPUT_ACTION_MOVE_DOWN,
    INPUT_ACTION_MOVE_LEFT,
    INPUT_ACTION_MOVE_RIGHT,
    INPUT_ACTION_MOVE_UP_LEFT,
    INPUT_ACTION_MOVE_UP_RIGHT,
    INPUT_ACTION_MOVE_DOWN_LEFT,
    INPUT_ACTION_MOVE_DOWN_RIGHT,
    INPUT_ACTION_COMMAND, // User typed a command
    INPUT_ACTION_VIEW_NORMAL,
    INPUT_ACTION_VIEW_SMELL,
    INPUT_ACTION_VIEW_SOUND,
    INPUT_ACTION_WAIT,
    INPUT_ACTION_QUIT,
    INPUT_ACTION_TIMEOUT
} InputAction;

typedef struct {
    InputAction type;
    char command_buffer[256];
} InputResult;

InputResult input_handle_key(int key);

// Command Parser
void input_parse_command(const char* cmd_str, Entity* player, EntityID* target_out);

#endif
