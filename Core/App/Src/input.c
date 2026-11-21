#include "FreeRTOS.h"
#include <stdbool.h>
#include "task.h"
#include "cmsis_os.h"
#include "queue.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal_uart.h"
#include "usart.h" // Can remove later

#include "input.h"

#define INPUT_QUEUE_SIZE 16

static StaticQueue_t inputQueueControlBlock;
osMessageQueueId_t inputQueue;
static uint8_t inputQueueBuffer[INPUT_QUEUE_SIZE * sizeof(InputEvent)];

const osMessageQueueAttr_t inputQueue_attributes = {
    .name = "inputQueue",
    .cb_mem = &inputQueueControlBlock,
    .cb_size = sizeof(inputQueueControlBlock),
    .mq_mem = &inputQueueBuffer,
    .mq_size = sizeof(inputQueueBuffer)
};

uint8_t input_char = 0;

void inputInit()
{
    inputQueue = osMessageQueueNew(INPUT_QUEUE_SIZE, sizeof(InputEvent), &inputQueue_attributes);
    HAL_UART_Receive_IT(&huart2, &input_char, 1);
}

// TODO: Change with button inputs
extern UART_HandleTypeDef huart2;
InputType input_type = 255;
InputState input_state = 255;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {

        if( input_type == 255 ) // If type not defined:
        {
            switch ( input_char ) {
                case( 'w' ): { input_type = INPUT_UP; } break;
                case( 'a' ): { input_type = INPUT_LEFT; } break;
                case( 's' ): { input_type = INPUT_DOWN; } break;
                case( 'd' ): { input_type = INPUT_RIGHT; } break;
                case( 'k' ): { input_type = INPUT_A; } break;
                case( 'l' ): { input_type = INPUT_B; } break;
            } // default: input_type still undefined
        }
        else
        {
            bool state_valid = true;

            switch ( input_char ) {
                case( 0 ): { input_state = INPUT_RELEASED; } break;
                case( 1 ): { input_state = INPUT_PRESSED; } break;
                default: { state_valid = false; } break;
            }
            
            if( state_valid )
            {
                InputEvent input_event = {
                    .type = input_type,
                    .state = input_state,
                };

                osMessageQueuePut(inputQueue, &input_event, 0, 0);

                input_type = 255; // Reset to empty
            }
        }
    }

    HAL_UART_Receive_IT(&huart2, &input_char, 1);
}
// ENDTODO