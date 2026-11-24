/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "radio.h"
#include "game.h"
#include "input.h"
#include "ssd1306.h"
#include "usart.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticMutexDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
uint32_t defaultTaskBuffer[ 256 ];
osStaticThreadDef_t defaultTaskControlBlock;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .cb_mem = &defaultTaskControlBlock,
  .cb_size = sizeof(defaultTaskControlBlock),
  .stack_mem = &defaultTaskBuffer[0],
  .stack_size = sizeof(defaultTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for radioReceiver */
osThreadId_t radioReceiverHandle;
uint32_t radioReceiverBuffer[ 256 ];
osStaticThreadDef_t radioReceiverControlBlock;
const osThreadAttr_t radioReceiver_attributes = {
  .name = "radioReceiver",
  .cb_mem = &radioReceiverControlBlock,
  .cb_size = sizeof(radioReceiverControlBlock),
  .stack_mem = &radioReceiverBuffer[0],
  .stack_size = sizeof(radioReceiverBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for radioTransmitte */
osThreadId_t radioTransmitteHandle;
uint32_t radioTransmitteBuffer[ 256 ];
osStaticThreadDef_t radioTransmitteControlBlock;
const osThreadAttr_t radioTransmitte_attributes = {
  .name = "radioTransmitte",
  .cb_mem = &radioTransmitteControlBlock,
  .cb_size = sizeof(radioTransmitteControlBlock),
  .stack_mem = &radioTransmitteBuffer[0],
  .stack_size = sizeof(radioTransmitteBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for inputProcessor */
osThreadId_t inputProcessorHandle;
uint32_t inputProcessorBuffer[ 128 ];
osStaticThreadDef_t inputProcessorControlBlock;
const osThreadAttr_t inputProcessor_attributes = {
  .name = "inputProcessor",
  .cb_mem = &inputProcessorControlBlock,
  .cb_size = sizeof(inputProcessorControlBlock),
  .stack_mem = &inputProcessorBuffer[0],
  .stack_size = sizeof(inputProcessorBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for radioReceived */
osMessageQueueId_t radioReceivedHandle;
uint8_t radioReceivedBuffer[ 16 * 17 ];
osStaticMessageQDef_t radioReceivedControlBlock;
const osMessageQueueAttr_t radioReceived_attributes = {
  .name = "radioReceived",
  .cb_mem = &radioReceivedControlBlock,
  .cb_size = sizeof(radioReceivedControlBlock),
  .mq_mem = &radioReceivedBuffer,
  .mq_size = sizeof(radioReceivedBuffer)
};
/* Definitions for radioTransmit */
osMessageQueueId_t radioTransmitHandle;
uint8_t radioTransmitBuffer[ 16 * 17 ];
osStaticMessageQDef_t radioTransmitControlBlock;
const osMessageQueueAttr_t radioTransmit_attributes = {
  .name = "radioTransmit",
  .cb_mem = &radioTransmitControlBlock,
  .cb_size = sizeof(radioTransmitControlBlock),
  .mq_mem = &radioTransmitBuffer,
  .mq_size = sizeof(radioTransmitBuffer)
};
/* Definitions for inputQueue */
osMessageQueueId_t inputQueueHandle;
uint8_t myQueue01Buffer[ 16 * 8 ];
osStaticMessageQDef_t myQueue01ControlBlock;
const osMessageQueueAttr_t inputQueue_attributes = {
  .name = "inputQueue",
  .cb_mem = &myQueue01ControlBlock,
  .cb_size = sizeof(myQueue01ControlBlock),
  .mq_mem = &myQueue01Buffer,
  .mq_size = sizeof(myQueue01Buffer)
};
/* Definitions for radioMutex */
osMutexId_t radioMutexHandle;
osStaticMutexDef_t radioMutexControlBlock;
const osMutexAttr_t radioMutex_attributes = {
  .name = "radioMutex",
  .cb_mem = &radioMutexControlBlock,
  .cb_size = sizeof(radioMutexControlBlock),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);
void startInputProcessor(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of radioMutex */
  radioMutexHandle = osMutexNew(&radioMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of radioReceived */
  radioReceivedHandle = osMessageQueueNew (16, 17, &radioReceived_attributes);

  /* creation of radioTransmit */
  radioTransmitHandle = osMessageQueueNew (16, 17, &radioTransmit_attributes);

  /* creation of inputQueue */
  inputQueueHandle = osMessageQueueNew (16, 8, &inputQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of radioReceiver */
  radioReceiverHandle = osThreadNew(StartTask02, NULL, &radioReceiver_attributes);

  /* creation of radioTransmitte */
  radioTransmitteHandle = osThreadNew(StartTask03, NULL, &radioTransmitte_attributes);

  /* creation of inputProcessor */
  inputProcessorHandle = osThreadNew(startInputProcessor, NULL, &inputProcessor_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* initialize radio module */
  radiohw_t hw = {
    .radioMutex = radioMutexHandle,
    .rxQueue    = radioReceivedHandle,
    .txQueue    = radioTransmitHandle,
    .spi        = &hspi2
  };
  radioInit( &hw );
  ssd1306_Init();

  gameInit();
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the radioReceiver thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  for(;;)
  {
    radioReceiverTaskRunner();
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the radioTransmitte thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for(;;)
  {
    radioTransmitterTaskRunner();
  }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_startInputProcessor */
/**
* @brief Function implementing the inputProcessor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startInputProcessor */
void startInputProcessor(void *argument)
{
  /* USER CODE BEGIN startInputProcessor */
  inputHw_t inputHw = {
    .pins = {
      .aPin     = INPUT_A_Pin,
      .bPin     = INPUT_B_Pin,
      .leftPin  = INPUT_LEFT_Pin,
      .rightPin = INPUT_RIGHT_Pin,
      .downPin  = INPUT_DOWN_Pin,
      .upPin    = INPUT_UP_Pin
    },
    .ports      = {
      .aPort     = INPUT_A_GPIO_Port,
      .bPort     = INPUT_B_GPIO_Port,
      .leftPort  = INPUT_LEFT_GPIO_Port,
      .rightPort = INPUT_RIGHT_GPIO_Port,
      .downPort  = INPUT_DOWN_GPIO_Port,
      .upPort    = INPUT_UP_GPIO_Port
    }
  };
  initInput( &inputHw );
  extern UART_HandleTypeDef huart2;
  char buf[32];
  /* Infinite loop */
  for(;;)
  {
    // inputEvent_t event;
    // if ( osMessageQueueGet( inputQueueHandle, &event, NULL, osWaitForever ) == osOK )
    // {
    //   sprintf( buf, "Key: %d, %s\r\n", event.type, event.state == INPUT_PRESSED ? "pressed": "released" );
    //   HAL_UART_Transmit( &huart2, (uint8_t*)buf, strlen(buf), 1000 );
    // }
    osDelay(1);
  }
  /* USER CODE END startInputProcessor */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

