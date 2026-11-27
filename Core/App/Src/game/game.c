#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "screen.h"
#include "input.h"
#include "state_stack.h"

#include "title.h"
#include "session.h"

#include "game.h"

#define TICKS_PER_SEC ( 50 )

GameState_t *(*stateTransitions[])(void) = {
    [STATE_NONE] = NULL,
    [STATE_TITLE] = getTitleGameState,
    [STATE_SESSION] = getSessionGameState
};

static void gameTask(void *argument)
{
    SpriteMap *map;
    GameState_t * currentState;
    uint32_t lastTicks = 0;
    inputEvent_t inputEvent;
    StateLabel_t queuedState;

    setState( getTitleGameState() );

    while(1)
    {
        getCurrentState(&currentState);
        if( currentState != NULL )
        {
            // TODO: REPLACE WITH INTERFACE FUNCTION
            extern osMessageQueueId_t inputQueueHandle;
            if( osMessageQueueGet(inputQueueHandle, &inputEvent, 0, 0 ) == osOK )
            {
                currentState->sendInput(&inputEvent);
            }
    
            currentState->update();
    
            map = currentState->render();
            screenDraw(*map);
            //screenDrawDebug(*map);
        }

        osDelayUntil( lastTicks + TICKS_PER_SEC );
        lastTicks = osKernelGetTickCount();

        queuedState = getQueuedState();
        if( queuedState != 0 )
        {
            setState( stateTransitions[queuedState]() );
            queuedState = 0;
        }
    }
}

osThreadId_t gameTaskHandle;
uint32_t gameTaskBuffer[ 512 ];
StaticTask_t gameTaskControlBlock;
const osThreadAttr_t gameTask_attributes = {
  .name = "gameTask",
  .cb_mem = &gameTaskControlBlock,
  .cb_size = sizeof(gameTaskControlBlock),
  .stack_mem = &gameTaskBuffer[0],
  .stack_size = sizeof(gameTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};

void gameInit()
{
    gameTaskHandle = osThreadNew(gameTask, NULL, &gameTask_attributes);
}