#include "input.h"
#include "cmsis_os2.h"
#include "main.h"
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

void initInput( inputHw_t * hw )
{
    if ( hw == NULL ) return;
    memcpy( &inputHw, hw, sizeof(inputHw_t) );

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
    
    inputType_t   pin;
    GPIO_TypeDef *port;
    if ( GPIO_Pin == inputHw.pins.upPin )
    {
        pin = INPUT_UP;
        port = INPUT_UP_GPIO_Port;
    }
    else if ( GPIO_Pin == inputHw.pins.downPin )
    {
        pin = INPUT_DOWN;
        port = INPUT_DOWN_GPIO_Port;
    }
    else if ( GPIO_Pin == inputHw.pins.leftPin )
    {
        pin = INPUT_LEFT;
        port = INPUT_LEFT_GPIO_Port;
    }
    else if ( GPIO_Pin == inputHw.pins.rightPin )
    {
        pin = INPUT_RIGHT;
        port = INPUT_RIGHT_GPIO_Port;
    }
    else if ( GPIO_Pin == inputHw.pins.aPin )
    {
        pin = INPUT_A;
        port = INPUT_A_GPIO_Port;
    }
    else if ( GPIO_Pin == inputHw.pins.bPin )
    {
        pin = INPUT_B;
        port = INPUT_B_GPIO_Port;
    }
    else
    {
        return; // no pin
    }

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

