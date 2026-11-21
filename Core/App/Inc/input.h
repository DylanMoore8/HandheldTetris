#include "gpio.h"
#include <stdint.h>

typedef struct 
{
    uint16_t      upPin;
    uint16_t      downPin;
    uint16_t      leftPin;
    uint16_t      rightPin;
    uint16_t      aPin;
    uint16_t      bPin;    
} pins_t;
typedef struct
{
    GPIO_TypeDef *upPort;
    GPIO_TypeDef *downPort;
    GPIO_TypeDef *leftPort;
    GPIO_TypeDef *rightPort;
    GPIO_TypeDef *aPort;
    GPIO_TypeDef *bPort;
} ports_t;

typedef struct 
{
    pins_t  pins;
    ports_t ports;
} inputHw_t;

typedef enum
{
    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_A,
    INPUT_B
} inputType_t;

typedef enum
{
    INPUT_PRESSED,
    INPUT_RELEASED
} inputState_t;

typedef struct 
{
    inputType_t type;
    inputState_t state;
} inputEvent_t;

void initInput( inputHw_t * hw );
