#include "game.h"
#include "input.h"
#include "screen.h"
#include "rng.h"
#include <string.h>
#include "state_stack.h"
#include "globals.h"


typedef struct
{
    const char * label;
    void (*callback)();
} MenuOption;

typedef enum
{
    TITLE_MAIN,
    TITLE_JOIN,
    TITLE_WAITING
} TitleState_t;

char * mainMenuOptions[] = {
    "SOLO",
    "MULTI",
    NULL
};

uint8_t cursorPosition;
uint8_t cursorMax;
static SpriteMap map;

char ** menuOptions = mainMenuOptions;
TitleState_t titleGameState;


// FUNCTIONS

void drawCursor()
{
    screenPutSprite(map, 4, 20 + (cursorPosition*3), 1);
}

void delegateSelect()
{
    switch(titleGameState)
    {
        case(TITLE_MAIN):
        {
            switch(cursorPosition)
            {
                case(0): // SOLO
                {
                    setGameMode(GAME_MODE_SOLO); // Set global game mode.
                    queueState(STATE_SESSION); // Launch the game.
                } break;
                case(1): // VS
                {
                    titleGameState = TITLE_JOIN;
                } break;
            }
        } break;
        case(TITLE_JOIN):
        {

        } break;
        default: {} break;
    }
}

void delegateBack()
{
    switch(titleGameState)
    {
        case(TITLE_JOIN):
        {
            titleGameState = TITLE_MAIN;
        } break;
        default: {} break;
    }
}

// CALLBACKS

void titleEnter()
{
    cursorPosition = 0;
    cursorMax = 2;
    titleGameState = TITLE_MAIN;
}

void titleSendInput( inputEvent_t *event )
{
    if( event->state == INPUT_PRESSED )
    {
        switch(event->type) {
            case(INPUT_A): {
                delegateSelect();
            } break;
            case(INPUT_B): {
                delegateBack();
            } break;
            case(INPUT_UP): {
                cursorPosition = (cursorPosition == 0) ? (0) : (cursorPosition - 1);
            } break;
            case(INPUT_DOWN): {
                cursorPosition = (cursorPosition == (cursorMax - 1)) ? (cursorMax - 1) : (cursorPosition + 1);
            } break;
            default: {} break;
        }
    }
}

void titleUpdate( )
{
    switch(titleGameState)
    {
        case(TITLE_MAIN):
        {

        } break;
        default: {} break;
    }
}

SpriteMap *titleRender( )
{
    screenReset(map);

    switch(titleGameState)
    {
        case(TITLE_MAIN):
        {
            screenPutText(map, 5, 5, "TETRIS");
            screenPutText(map, 4, 8, "DYLAN M.");
            screenPutText(map, 4, 11, "CALLUM M.");

            screenPutText(map, 6, 20, "SOLO");
            screenPutText(map, 6, 23, "VS");
            drawCursor();
        } break;
        case(TITLE_JOIN):
        {
            screenPutText(map, 6, 5, "ROOMS");
        } break;
        default: {} break;
    }

    return &map;
}

GameState_t titleState = {
    .enter = titleEnter,
    .sendInput = titleSendInput,
    .update = titleUpdate,
    .render = titleRender,
};

GameState_t *getTitleGameState()
{
    return &titleState;
}