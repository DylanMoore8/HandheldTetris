#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

#define SCREEN_SPRITE_WIDTH ( 64 / 4 )
#define SCREEN_SPRITE_HEIGHT ( 128 / 4 )

#define SPRITE_EMPTY ( 0 )

typedef uint8_t SpriteIndex;
typedef SpriteIndex SpriteMap[SCREEN_SPRITE_HEIGHT][SCREEN_SPRITE_WIDTH];

void screenReset(SpriteMap map);

void screenPutSprite(SpriteMap map, uint8_t x, uint8_t y, SpriteIndex sprite_inx);

void screenDrawDebug(const SpriteMap map);
void screenDrawBFDebug(const SpriteMap back, const SpriteMap fore);

#endif