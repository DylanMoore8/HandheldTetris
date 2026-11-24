#ifndef SPRITES_U
#define SPRITES_U

#include <stdint.h>

#define SpA_U 0x20
#define SpA_L 0x30
#define SpB_U 0x21
#define SpB_L 0x31
#define SpC_U 0x22
#define SpC_L 0x32
#define SpD_U 0x23
#define SpD_L 0x33
#define SpE_U 0x24
#define SpE_L 0x34
#define SpF_U 0x25
#define SpF_L 0x35
#define SpG_U 0x26
#define SpG_L 0x36
#define SpH_U 0x27
#define SpH_L 0x37
#define SpI_U 0x28
#define SpI_L 0x38
#define SpJ_U 0x29
#define SpJ_L 0x39
#define SpK_U 0x2A
#define SpK_L 0x3A
#define SpL_U 0x2B
#define SpL_L 0x3B
#define SpM_U 0x2C
#define SpM_L 0x3C
#define SpN_U 0x2D
#define SpN_L 0x3D
#define SpO_U 0x2E
#define SpO_L 0x3E
#define SpP_U 0x2F
#define SpP_L 0x3F
#define SpQ_U 0x40
#define SpQ_L 0x50
#define SpR_U 0x41
#define SpR_L 0x51
#define SpS_U 0x42
#define SpS_L 0x52
#define SpT_U 0x43
#define SpT_L 0x53
#define SpU_U 0x44
#define SpU_L 0x54
#define SpV_U 0x45
#define SpV_L 0x55
#define SpW_U 0x46
#define SpW_L 0x56
#define SpX_U 0x47
#define SpX_L 0x57
#define SpY_U 0x48
#define SpY_L 0x58
#define SpZ_U 0x49
#define SpZ_L 0x59

#define SpCLN 0x5A

#define Sp0_U 0x60 
#define Sp0_L 0x70
#define Sp1_U 0x61 
#define Sp1_L 0x71
#define Sp2_U 0x62 
#define Sp2_L 0x72
#define Sp3_U 0x63 
#define Sp3_L 0x73
#define Sp4_U 0x64 
#define Sp4_L 0x74
#define Sp5_U 0x65 
#define Sp5_L 0x75
#define Sp6_U 0x66 
#define Sp6_L 0x76
#define Sp7_U 0x67 
#define Sp7_L 0x77
#define Sp8_U 0x68 
#define Sp8_L 0x78
#define Sp9_U 0x69 
#define Sp9_L 0x79

#define NUMBER_OF_SPRITES ( 128 )

typedef uint8_t Sprite[2];

/**
 * @brief Get sprite data from index
 * 
 * @param[in] index Index of sprite.
 * @param[out] sprite Sprite data.
 */
const Sprite *getSprite(uint8_t index);

#endif