# Radio module example

This is the beacon and opponent-discovery demo that ran in `StartDefaultTask`
(`Core/Src/freertos.c`) on the `feature/radio` branch before it was merged with
master. It was taken out of the firmware because its UART prints share `huart2`
with the screen emulator. It is kept here for reference.

## What it does

1. Initializes the radio, using the RTOS objects that CubeMX generates in
   `freertos.c` (`radioMutex`, the `radioReceived`/`radioTransmit` queues) and `hspi2`.
2. Once a second:
   - fetches the list of opponents discovered so far (`radioGetAvailableOpponents`)
   - broadcasts a beacon with this board's display name (`radioBroadcast`)
   - prints each discovered opponent's name over `huart2`

The `radioReceiver` (`StartTask02`) and `radioTransmitte` (`StartTask03`) tasks must
also be running, since they call `radioReceiverTaskRunner()` and
`radioTransmitterTaskRunner()`.

The display name defaults to `"JOE"`. It is overridden by `DISPLAY_NAME`, which the
root `CMakeLists.txt` defines from `config.json` (`"useName": true`, `"name": "..."`).
`BOARD_ID` comes from the same file.

## Code

The original body of `StartDefaultTask`. It needs `radio.h`, `usart.h`, `stdio.h`
and `string.h`, which are already included in `freertos.c`.

```c
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
```

## Running it alongside the game

To try it without removing `initGame()`, move the loop into its own task. Otherwise,
temporarily swap it into `StartDefaultTask`. In either case, close the screen
emulator first, because the UART output will interleave with its sprite stream.
