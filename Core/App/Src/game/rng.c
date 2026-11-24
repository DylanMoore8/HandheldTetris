#include "rng.h"
#include <stdint.h>

// Pulled from https://en.wikipedia.org/wiki/Permuted_congruential_generator

static uint64_t       state      = 0x4d595df4d0f33173;		// Or something seed-dependent
static uint64_t const multiplier = 6364136223846793005u;
static uint64_t const increment  = 1442695040888963407u;	// Or an arbitrary odd constant

static uint32_t rotr32(uint32_t x, unsigned r)
{
	return x >> r | x << (-r & 31);
}

uint32_t pcg32(void)
{
	uint64_t x = state;
	unsigned count = (unsigned)(x >> 59);		// 59 = 64 - 5

	state = x * multiplier + increment;
	x ^= x >> 18;								// 18 = (64 - 27)/2
	return rotr32((uint32_t)(x >> 27), count);	// 27 = 32 - 5
}

void pcg32_init(uint64_t seed)
{
	state = seed + increment;
	(void)pcg32();
}

/* obtained from an online example */
uint64_t getSeed( ADC_HandleTypeDef *hadc )
{
    uint32_t noise = 0;
    HAL_ADC_Start(hadc);
    for (int i = 0; i < 4; i++) {
        HAL_ADC_PollForConversion(hadc, 10);
        noise ^= HAL_ADC_GetValue(hadc) << (i * 8);
    }
    HAL_ADC_Stop(hadc);
    return noise;
}