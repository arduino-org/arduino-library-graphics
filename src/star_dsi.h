#ifndef _STAR_DSI_H
#define _STAR_DSI_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "otm8009a.h"
#include "stm32f4xx_hal.h"

#define LCD_OK                  0x00
#define LCD_ERROR               0x01
#define LCD_TIMEOUT             0x02

typedef enum {
  LCD_ORIENTATION_PORTRAIT  = 0x00, // Portrait
  LCD_ORIENTATION_LANDSCAPE = 0x01, // Landscape
  LCD_ORIENTATION_INVALID   = 0x02  // Invalid
} LCD_OrientationTypeDef;

typedef enum {
  LCD_DSI_PIXEL_DATA_FMT_RBG888  = 0x00, // RGB888 : 24 bpp
  LCD_DSI_PIXEL_DATA_FMT_RBG565  = 0x02, // RGB565 : 16 bpp
  LCD_DSI_PIXEL_DATA_FMT_INVALID = 0x03  // Invalid

} LCD_DsiPixelDataFmtTypeDef;

extern void DSI_IO_WriteCmd(uint32_t NbrParams, uint8_t *pParams);
extern uint8_t STAR_DSI_Init(LCD_OrientationTypeDef orientation);
extern uint16_t STAR_DSI_PanelWidth(void);
extern uint16_t STAR_DSI_PanelHeight(void);
extern void STAR_DSI_SetDrawScreen(uint8_t scrIdx);
extern uint8_t STAR_DSI_GetDrawScreen(void);
extern void STAR_DSI_ShowScreen(uint8_t scrIdx);
extern void STAR_DSI_CopyScreen(uint8_t fromIdx, uint8_t toIdx);
extern void STAR_DSI_DrawPoint(uint16_t x, uint16_t y, uint32_t color);
extern void STAR_DSI_FillRectDma(uint16_t x, uint16_t y, uint16_t width,
        uint16_t height, uint32_t color);
extern void STAR_DSI_DisplayOn(void);
extern void STAR_DSI_DisplayOff(void);

#ifdef __cplusplus
}
#endif

#endif // _STAR_DSI_H
