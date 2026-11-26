#ifndef RNG_H
#define RNG_H

#include "adc.h"

#include <stdint.h>

typedef struct
{
    uint64_t state;
} Pcg32State_t;

/**
 * @brief Generates a random number
 * 
 * @return A random 32-bit number.
 */
uint32_t pcg32(Pcg32State_t *state);

/**
 * @brief Initializes the random number generator.
 * 
 */
void pcg32_init(Pcg32State_t *state, uint64_t seed);

/**
 * gets seed from ADC
 */
uint64_t getSeed( ADC_HandleTypeDef *hadc );

#endif