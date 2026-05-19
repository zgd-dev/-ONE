#ifndef __LCD_H
#define __LCD_H

#include "stm32f4xx_hal.h"
#include "pin_config.h"



void LCD_Init(void);
void LCD_Clear(uint16_t color);
void LCD_SetWindows(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawBigPoint(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_ShowString(uint16_t x, uint16_t y, char *str, uint16_t color);
void LCD_WritePixels(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *color_p, uint32_t pixel_count);
#endif