#ifndef GLOBALS_H
#define GLOBALS_H

typedef enum
{
    GAME_MODE_SOLO,
    GAME_MODE_VS
} GameMode_t;

/**
 * @brief Set the Game Mode object
 * 
 * @param gm Game mode
 */
void setGameMode( GameMode_t gm );

/**
 * @brief Get the Game Mode object
 * 
 * @return GameMode_t Game mode.
 */
GameMode_t getGameMode();

/**
 * @brief Sets the seed
 * 
 * @param s Seed
 */
void setGameSeed( uint64_t s );

/**
 * @brief Gets the seed.
 * 
 * @return uint64_t Seed
 */
uint64_t getGameSeed();

#endif