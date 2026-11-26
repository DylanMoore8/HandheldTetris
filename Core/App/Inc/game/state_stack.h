#ifndef STATE_STACK_H
#define STATE_STACK_H

#include "input.h"
#include "screen.h"

typedef enum
{
    STATE_NONE,
    STATE_TITLE,
    STATE_SESSION,
} StateLabel_t;

typedef struct
{
    void (*enter)( );
    void (*sendInput)( inputEvent_t *event );
    void (*update)();
    SpriteMap *(*render)();
} GameState_t;

bool stateStackEmpty(  );
bool getCurrentState( GameState_t ** state );

void setState( GameState_t * state );

void queueState( StateLabel_t state );
StateLabel_t getQueuedState( );

#endif