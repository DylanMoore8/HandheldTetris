#include "input.h"
#include "cmsis_os2.h"
#include "stm32f411xe.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_gpio.h"
#include <stdint.h>
#include "stdbool.h"
#include "string.h"
#include <limits.h>

extern osMessageQueueId_t inputQueueHandle;

static inputHw_t inputHw = {0};
static bool initialized = false;

#define N_PINS   ( sizeof(inputHw.pins) / sizeof(inputHw.pins.aPin) )
/* one slot per bit in a pin mask, so every pin gets a unique index */
#define PIN_BITS ( sizeof(inputHw.pins.aPin) * CHAR_BIT )
static inputType_t   pinsMap[PIN_BITS]  = {0};
static GPIO_TypeDef *portsMap[PIN_BITS] = {0};

static uint8_t pinToIndex( uint16_t pin )
{
    uint8_t idx = 0;
    /* shift right until the set bit reaches bit 0, counting the shifts */
    while ( pin > 1 )
    {
        pin = (pin >> 1);
        idx++;
    }
    return idx;
}

void initInput( inputHw_t * hw )
{
    if ( hw == NULL ) return;
    memcpy(&inputHw, hw, sizeof(inputHw_t));
    
    /* build hash map */
    uint16_t      pins[N_PINS]     = {0};
    GPIO_TypeDef *ports[N_PINS]    = {0};

    memcpy(pins, &inputHw.pins, N_PINS*sizeof(inputHw.pins.aPin));
    memcpy(ports, &inputHw.ports, N_PINS*sizeof(inputHw.ports.aPort));

    for ( int i = 0; i < N_PINS; i++ )
    {
        /* our key is the high bit */
        uint8_t idx   = pinToIndex( pins[i] );
        pinsMap[idx]  = (inputType_t)i;
        portsMap[idx] = ports[i];
    }

    initialized = true;
}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if ( !initialized ) return;
    /** get the port, stm forces no conflicts on the bit masks, 
     *  however I think there is still a better way this can be done. 
     *  Note: The pins must me initialzied to trigger on the interrupt  
     */
    uint8_t       idx  = pinToIndex( GPIO_Pin );
    inputType_t   pin  = pinsMap[idx];
    GPIO_TypeDef *port = portsMap[idx];
    if ( port == NULL ) return;

    bool isPressed = (HAL_GPIO_ReadPin(port, GPIO_Pin) == GPIO_PIN_RESET);

    /* build our key press event data structure */
    inputEvent_t event = {
        .type  = pin,
        .state = isPressed ? INPUT_PRESSED : INPUT_RELEASED
    };

    /* send to queue */
    if ( osMessageQueuePut( inputQueueHandle, &event, 0, 0 ) == osOK )
    {
        // ok
    }
}

