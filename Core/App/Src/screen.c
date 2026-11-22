#include "screen.h"
#include "stm32f4xx.h"
#include <string.h>
#include <stdio.h>

void screenReset(SpriteMap map)
{
    memset(map, 0x00, SCREEN_SPRITE_WIDTH * SCREEN_SPRITE_HEIGHT);
}

void screenPutSprite(SpriteMap map, uint8_t x, uint8_t y, SpriteIndex sprite_inx)
{
    map[y][x] = sprite_inx;
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

    #define LINE_LENGTH ( (SCREEN_SPRITE_WIDTH*2) + 2 )

    char line[LINE_LENGTH];
    
    for(int y = 0; y < SCREEN_SPRITE_HEIGHT; y++)
    {
        for(int x = 0; x < SCREEN_SPRITE_WIDTH; x++)
        {
            sprintf(&line[x*2], "%02X", map[y][x]);
        }

        HAL_UART_Transmit(&huart2, (uint8_t *)line, (SCREEN_SPRITE_WIDTH*2), HAL_MAX_DELAY);
    }
    HAL_UART_Transmit(&huart2, (uint8_t *)"\n", 1, HAL_MAX_DELAY);
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

    #define LINE_LENGTH ( (SCREEN_SPRITE_WIDTH*2) + 2 )

    char line[LINE_LENGTH];
    
    for(int y = 0; y < SCREEN_SPRITE_HEIGHT; y++)
    {
        for(int x = 0; x < SCREEN_SPRITE_WIDTH; x++)
        {
            SpriteIndex si = front[y][x];
            if( si == SPRITE_EMPTY )
            {
                si = back[y][x];
                
            }
            sprintf(&line[x*2], "%02X", si);
        }

        HAL_UART_Transmit(&huart2, (uint8_t *)line, (SCREEN_SPRITE_WIDTH*2), HAL_MAX_DELAY);
    }
    HAL_UART_Transmit(&huart2, (uint8_t *)"\n", 1, HAL_MAX_DELAY);
}