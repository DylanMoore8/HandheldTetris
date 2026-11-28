
#include <stdint.h>
#include "globals.h"

static GameMode_t gameMode;
void setGameMode( GameMode_t gm )
{
    gameMode = gm;
}

GameMode_t getGameMode()
{
    return gameMode;
}

static uint64_t seed;
void setGameSeed( uint64_t s )
{
    seed = s;
}

uint64_t getGameSeed()
{
    return seed;
} 