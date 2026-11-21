#ifndef INPUT_H
#define INPUT_H

typedef enum
{
    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_A,
    INPUT_B,
} InputType;

typedef enum
{
    INPUT_PRESSED,
    INPUT_RELEASED,
} InputState;

typedef struct
{
    InputType type;
    InputState state;
} InputEvent;

void inputInit();

#endif