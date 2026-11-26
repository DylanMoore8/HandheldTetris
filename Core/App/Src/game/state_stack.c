// I want a system where I can isolate states
// All states would have functions that 

#include <stdbool.h>
#include <stdint.h>
#include "screen.h"
#include "input.h"
#include "game.h"

#include "state_stack.h"

// STATES

#define STATE_STACK_SIZE ( 8 )

GameState_t *currentState = NULL;
StateLabel_t queuedState = STATE_NONE;

bool getCurrentState( GameState_t ** state )
{
    bool rc = false;

    if( currentState != NULL )
    {
        *state = currentState;
        rc = true;
    }

    return rc;
}

void setState( GameState_t * state )
{
    state->enter();
    currentState = state;
    queuedState = 0;
}

void queueState( StateLabel_t state )
{
    queuedState = state;
}

StateLabel_t getQueuedState( )
{
    return queuedState;
}