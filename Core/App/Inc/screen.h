#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include "sprites.h"

#define SCREEN_SPRITE_WIDTH ( 64 / 4 )
#define SCREEN_SPRITE_HEIGHT ( 128 / 4 )

typedef SpriteIndex SpriteMap[SCREEN_SPRITE_HEIGHT][SCREEN_SPRITE_WIDTH];

void screenReset(SpriteMap map);

bool swapSpriteBuffers( uint32_t timeoutMs );
SpriteMap * screenGetWorkingBuffer( void );

void screenPutSprite(SpriteMap map, uint8_t x, uint8_t y, SpriteIndex sprite_inx);

void screenPutText(SpriteMap map, uint8_t x, uint8_t y, char * text);
void screenPutNumber(SpriteMap map, uint8_t x, uint8_t y, uint32_t num);
void screenOverlay( SpriteMap cmb, const SpriteMap back, const SpriteMap front );

void screenDraw(const SpriteMap map);
void screenDrawBF(const SpriteMap back, const SpriteMap front);

// Prints to USART for screen emulator
void screenDrawDebug(const SpriteMap map);
void screenDrawBFDebug(const SpriteMap back, const SpriteMap fore);

void screenInit(void);

#endif