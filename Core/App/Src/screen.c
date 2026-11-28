#include "screen.h"
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "ssd1306.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "sprites.h"

static uint8_t screenBuffer[SSD1306_BUFFER_SIZE];

static SpriteMap spriteBuffers[2];
static uint8_t workingIndex = 0;

static SpriteMap * workingBuffer = &spriteBuffers[0];
static SpriteMap * drawBuffer = &spriteBuffers[1];

static osSemaphoreId_t drawStart;
static osSemaphoreId_t drawDone;

static char numberBuffer[16];

bool swapSpriteBuffers( uint32_t timeoutMs )
{
    if( osSemaphoreAcquire(drawDone, timeoutMs) != osOK )
    {
        return false;
    }

    // Perform the swap
    workingIndex = 1 - workingIndex;
    workingBuffer = &spriteBuffers[workingIndex];
    drawBuffer = &spriteBuffers[1 - workingIndex];

    osSemaphoreRelease( drawStart );

    return true;
}

SpriteMap * screenGetWorkingBuffer( void )
{
    return workingBuffer;
}

void screenReset(SpriteMap map)
{
    memset(map, 0x00, SCREEN_SPRITE_WIDTH * SCREEN_SPRITE_HEIGHT);
}

void screenPutSprite(SpriteMap map, uint8_t x, uint8_t y, SpriteIndex sprite_inx)
{
    map[y][x] = sprite_inx;
}

void screenPutText(SpriteMap map, uint8_t x, uint8_t y, char * text)
{
    if( (y + 1) < SCREEN_SPRITE_HEIGHT ) // If the bottom of text is not clipping bottom of screen.
    {
        uint8_t i = 0;
        uint8_t cur_x = x;

        char c = text[i];
        while( ( c != 0 ) && ( cur_x < SCREEN_SPRITE_WIDTH ) )
        {
            SpriteCharacter_t sprChar;
            if( getSpriteFromChar(c, &sprChar) )
            {
                map[ y ][cur_x] = sprChar.top;
                map[y+1][cur_x] = sprChar.bottom;
            }

            i++;
            cur_x++;

            c = text[i];
        }
    }
}

void screenPutNumber(SpriteMap map, uint8_t x, uint8_t y, uint32_t num)
{
    sprintf(numberBuffer, "%lu", num);
    screenPutText(map, x, y, numberBuffer);
}

// So, with the scheme of the ssd1306, bytes are written as so:
/*
    ------------------------------------------------------------------
    |01(127)7                                                         |
    |01(126)7                                                         |
    |01(125)7                                                         |
    |01(124)7                                                         |
    |01(123)7                                                         |
    |01(122)7                                                         |
                                    ...
    |01(2)567     /\                                                  |
    |01(1)567     ||                                                  |
    |01(0)567 01(128)7                                                |
    ------------------------------------------------------------------

Where 0 is the MSB, 7 is LSB, (n) is buffer[n].                               

My thought is to draw **two sprites at a time (2x1)** like so:

    0 1 2 3|4 5 6 7 (3)
    0 1 2 3|4 5 6 7 (2)
    0 1 2 3|4 5 6 7 (1)
    0 1 2 3|4 5 6 7 (0)

or

    sprite1 | sprite2

Where boundaries define the sprites. Every 2x1 grid of sprites would be processed from 
bottom-left to top-right.

Sprites are organized as so: every nibble is stacked sequentially from top to bottom.
So for the sprite:

    @@@@
    @..@
    @..@
    @@@@

Would be defined as:

    0123 \ of byte
    4567 /    0
    0123 \ of byte
    4567 /    1

Together:

    Sp1  | Sp2
    0123 | 0123 \ of byte
    4567 | 4567 /    0
    0123 | 0123 \ of byte
    4567 | 4567 /    1

So here's how sprite1 and sprite2 should be sent:

    [MSN, byte 0, sprite 2] | [MSN, byte 0, sprite 1],
    [LSN, byte 0, sprite 2] | [LSN, byte 0, sprite 1],
    [MSN, byte 0, sprite 2] | [MSN, byte 0, sprite 1],
    [LSN, byte 0, sprite 2] | [LSN, byte 0, sprite 1],

Where MSN is most significant nibble, LSN is least significant nibble.

Converting that into operations:

    [byte 1, sprite 2] & 0xF0 | [byte 1, sprite 1] >> 4,
    [byte 1, sprite 2] >> 4   | [byte 1, sprite 1] & 0x0F,
    [byte 0, sprite 2] & 0xF0 | [byte 0, sprite 1] << 4,
    [byte 0, sprite 2] << 4   | [byte 0, sprite 1] & 0x0F,
*/
void screenDraw(const SpriteMap map)
{
    uint8_t spriteX = 0;
    uint8_t spriteY = SCREEN_SPRITE_HEIGHT - 1;

    const Sprite *sprite1;
    const Sprite *sprite2;

    for( uint32_t i = 0; i < SSD1306_BUFFER_SIZE; i += 4 )
    {
        sprite1 = getSprite(map[spriteY][spriteX  ]);
        sprite2 = getSprite(map[spriteY][spriteX+1]);

        screenBuffer[ i+3 ] = ( (*sprite2)[0] << 4 )   | ( (*sprite1)[0] & 0x0F );
        screenBuffer[ i+2 ] = ( (*sprite2)[0] & 0xF0 ) | ( (*sprite1)[0] >> 4 );
        screenBuffer[ i+1 ] = ( (*sprite2)[1] << 4 )   | ( (*sprite1)[1] & 0x0F );
        screenBuffer[  i  ] = ( (*sprite2)[1] & 0xF0 ) | ( (*sprite1)[1] >> 4 );

        if( spriteY != 0 )
        {
            spriteY--;
        }
        else
        {
            spriteX += 2;
            spriteY = SCREEN_SPRITE_HEIGHT - 1;
        }
    }

    ssd1306_WriteBuffer(screenBuffer);
}

void screenOverlay( SpriteMap cmb, const SpriteMap back, const SpriteMap front )
{
    for(int y = 0; y < SCREEN_SPRITE_HEIGHT; y++)
    {
        for(int x = 0; x < SCREEN_SPRITE_WIDTH; x++)
        {
            SpriteIndex si = front[y][x];
            if( si == SPRITE_EMPTY )
            {
                si = back[y][x];
                
            }

            cmb[y][x] = si;
        }
    }
}

void screenDrawDebug(const SpriteMap map)
{
    // TODO: Change to SSD1306
    extern UART_HandleTypeDef huart2;
    extern HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout);
    // Format:
    // 00112233445566778899AABBCCDDEEFF\n
    // 00112233445566778899AABBCCDDEEFF\n
    // ...

    HAL_UART_Transmit(&huart2, (uint8_t *)map, (SCREEN_SPRITE_WIDTH * SCREEN_SPRITE_HEIGHT), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart2, &(uint8_t){ 0xFF }, 1, HAL_MAX_DELAY);
}

void screenDrawBFDebug(const SpriteMap back, const SpriteMap front)
{
    // TODO: Change to SSD1306
    extern UART_HandleTypeDef huart2;
    extern HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout);
    // Format:
    // 00112233445566778899AABBCCDDEEFF\n
    // 00112233445566778899AABBCCDDEEFF\n
    // ...
    
    for(int y = 0; y < SCREEN_SPRITE_HEIGHT; y++)
    {
        for(int x = 0; x < SCREEN_SPRITE_WIDTH; x++)
        {
            SpriteIndex si = front[y][x];
            if( si == SPRITE_EMPTY )
            {
                si = back[y][x];
                
            }
            //sprintf(&line[x*2], "%02X", si);
            HAL_UART_Transmit(&huart2, (uint8_t *)&si, 1, HAL_MAX_DELAY);
        }
    }
    HAL_UART_Transmit(&huart2, &(uint8_t){ 0xFF }, 1, HAL_MAX_DELAY);
}

osThreadId_t screenTaskHandle;
uint32_t screenTaskBuffer[ 512 ];
StaticTask_t screenTaskControlBlock;
const osThreadAttr_t screenTask_attributes = {
  .name = "screenTask",
  .cb_mem = &screenTaskControlBlock,
  .cb_size = sizeof(screenTaskControlBlock),
  .stack_mem = &screenTaskBuffer[0],
  .stack_size = sizeof(screenTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};

/*
There's two buffers:
 - A working buffer
 - A draw buffer

The working buffer is worked on by the application.
While that is happending, the draw buffer is being drawn by this task.

When the draw buffer and the working buffer are done being worked on and drawn,
They will switch.
*/
void screenTask(void *argument)
{
    while(1)
    {
        osSemaphoreAcquire(drawStart, osWaitForever);
    
        screenDraw(*drawBuffer);
        screenDrawDebug(*drawBuffer); // Print to USART
    
        osSemaphoreRelease( drawDone );
    }
}

void screenInit(void)
{
    drawStart = osSemaphoreNew( 1, 0, NULL );
    drawDone  = osSemaphoreNew( 1, 1, NULL );
    screenTaskHandle = osThreadNew(screenTask, NULL, &screenTask_attributes);
}