#include "game.h"
#include "input.h"
#include "screen.h"
#include "rng.h"
#include "radio.h"
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

char ** menuOptions = mainMenuOptions;
TitleState_t titleGameState;

availableOpponent ops[MAX_OPPONENTS];
availableOpponent selectedOpponent;

extern ADC_HandleTypeDef hadc1;
static uint64_t seed;
static uint64_t sessionSeed;

static availableOpponent available_ops[MAX_OPPONENTS];
uint8_t available_ops_length;
static char available_opp_names[4][MAX_OPPONENTS];

// FUNCTIONS

void drawCursor( SpriteMap * map )
{
    screenPutSprite(*map, 4, 20 + (cursorPosition*3), 1);
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
                    cursorPosition = 0; // reset cursor
                } break;
            }
        } break;
        case(TITLE_JOIN):
        {
            if( available_ops_length > 0 && ( cursorPosition < available_ops_length ) ) // Safety check
            {
                // Store the selected opponent.
                memcpy(&selectedOpponent, &available_ops[cursorPosition], sizeof(availableOpponent));
                titleGameState = TITLE_WAITING;
            }
        } break;
        case(TITLE_WAITING):
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
            titleGameState = TITLE_MAIN; // Go back to main
            available_ops_length = 0;
            cursorMax = 2;
            cursorPosition = 0;
        } break;
        case(TITLE_WAITING):
        {
            cursorMax = 1; // Lock the cursor
            cursorPosition = 0;
            titleGameState = TITLE_JOIN;
        }
        default: {} break;
    }
}

// CALLBACKS

void titleEnter()
{
    seed = getSeed( &hadc1 );

    cursorPosition = 0;
    cursorMax = 2;
    titleGameState = TITLE_MAIN;

    selectedOpponent = (availableOpponent){ .id = 0 }; 

    radioFlushData();

    memset(available_opp_names, 0, MAX_OPPONENTS * 4);
    memset(available_ops, 0, MAX_OPPONENTS * sizeof(availableOpponent));
    available_ops_length = 0;
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
    // If we are in the join or waiting screen:
    if( (titleGameState == TITLE_JOIN) || (titleGameState == TITLE_WAITING) )
    {
        // we are here 
        radioBroadcast( DISPLAY_NAME, seed );
        radioGetAvailableOpponents( ops, MAX_OPPONENTS );
    }

    switch(titleGameState)
    {
        case(TITLE_JOIN):
        {
            // who is out there?

            available_ops_length = 0;
            for ( uint8_t i = 0; i < MAX_OPPONENTS; i++ )
            {
                availableOpponent op = ops[i];
                char * nextNameBuffer = available_opp_names[available_ops_length];
                availableOpponent * nextAvailable = &available_ops[available_ops_length];

                if( op.id != 0 )
                {
                    memcpy(nextAvailable, &op, sizeof(availableOpponent) );
                    memcpy(nextNameBuffer, op.info.displayName, 3); // nameBuffer is of length 4, where nameBuffer[3] == '\0'.
                    available_ops_length++;
                }
            }

            cursorMax = available_ops_length;

            if( cursorMax == 0 )
            {
                cursorPosition = 0;
            }
            else if( cursorPosition >= cursorMax )
            {
                cursorPosition = cursorMax - 1;
            }

        } break;
        case(TITLE_WAITING):
        {
            if ( radioSessionRequestJoin( selectedOpponent, seed, &sessionSeed ) == RADIO_OK )
            {
                setGameSeed(sessionSeed);

                // enter session
                setGameMode( GAME_MODE_VS );
                queueState( STATE_SESSION );
            }
        } break;
        default: {} break;
    }
}

void titleRender( SpriteMap * map )
{
    screenReset(*map);

    switch(titleGameState)
    {
        case(TITLE_MAIN):
        {
            screenPutText(*map, 5, 5, "TETRIS");
            screenPutText(*map, 4, 8, "DYLAN M.");
            screenPutText(*map, 4, 11, "CALLUM M.");

            screenPutText(*map, 6, 20, "SOLO");
            screenPutText(*map, 6, 23, "VS");
            drawCursor(map);
        } break;
        case(TITLE_JOIN):
        {
            screenPutText(*map, 6, 5, "ROOMS");

            for( int i = 0; i < available_ops_length; i++ )
            {
                screenPutText(*map, 6, 20 + (i*3), available_opp_names[i] );
            }
            
            if( available_ops_length != 0 )
            {
                drawCursor(map);
            }
            
        } break;
        case(TITLE_WAITING):
        {
            screenPutText(*map, 3, 8, "WAITING...");
        } break;
        default: {} break;
    }
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