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
  .priority = (osPriority_t) osPriorityLow,
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
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for radioReceived */
osMessageQueueId_t radioReceivedHandle;
uint8_t radioReceivedBuffer[ 16 * 9 ];
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
uint8_t radioTransmitBuffer[ 16 * 9 ];
osStaticMessageQDef_t radioTransmitControlBlock;
const osMessageQueueAttr_t radioTransmit_attributes = {
  .name = "radioTransmit",
  .cb_mem = &radioTransmitControlBlock,
  .cb_size = sizeof(radioTransmitControlBlock),
  .mq_mem = &radioTransmitBuffer,
  .mq_size = sizeof(radioTransmitBuffer)
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
  radioReceivedHandle = osMessageQueueNew (16, 9, &radioReceived_attributes);

  /* creation of radioTransmit */
  radioTransmitHandle = osMessageQueueNew (16, 9, &radioTransmit_attributes);

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

  char str1[] = "Radio initialized\r\n";
  int len1    = strlen(str1); 
  HAL_UART_Transmit( &huart2, (uint8_t*)str1, len1, HAL_MAX_DELAY );

  char name[DISPLAY_NAME_LEN] = "JOE";

  /* Infinite loop */
  char str2[] = "Beacon sent\r\n";
  char strbuf[32];
  availableOpponent ops[MAX_OPPONENTS] = {0};
  for(;;)
  {
    radioGetAvailableOpponents( ops, MAX_OPPONENTS );

    #ifdef DISPLAY_NAME
      memcpy(name, DISPLAY_NAME, DISPLAY_NAME_LEN);
    #endif

    radioBroadcast( name );
    HAL_UART_Transmit( &huart2, (uint8_t*)str2, strlen(str2), HAL_MAX_DELAY );
    for (int i = 0; i < MAX_OPPONENTS; i++ )
    {
      availableOpponent op = ops[i];
      if ( op.id != 0 )
      {
        sprintf( strbuf, "Opponent %.*s discovered\r\n",DISPLAY_NAME_LEN, op.info.displayName );
        HAL_UART_Transmit( &huart2, (uint8_t*)strbuf, strlen(strbuf), HAL_MAX_DELAY );
      }
    }
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

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

