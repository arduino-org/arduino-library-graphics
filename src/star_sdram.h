#ifndef _STAR_SDRAM_H
#define _STAR_SDRAM_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"

#define SDRAM_OK            ((uint8_t)0x00)
#define SDRAM_ERROR         ((uint8_t)0x01)
#define SDRAM_DEVICE_ADDR   ((uint32_t)0xC0000000)
#define SDRAM_DEVICE_SIZE   ((uint32_t)0x800000)

extern uint8_t STAR_SDRAM_Init(void);
extern uint8_t STAR_SDRAM_Test(void);

#ifdef __cplusplus
}
#endif

#endif // _STAR_SDRAM_H

