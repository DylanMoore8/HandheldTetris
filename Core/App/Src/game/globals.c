
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
