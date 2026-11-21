#ifndef RNG_H
#define RNG_H

#include <stdint.h>
/**
 * @brief Generates a random number
 * 
 * @return A random 32-bit number.
 */
uint32_t pcg32(void);

/**
 * @brief Initializes the random number generator.
 * 
 */
void pcg32_init(uint64_t seed);


#endif