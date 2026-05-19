#ifndef __TOUCH_H
#define __TOUCH_H

#include "stm32f4xx_hal.h"


// LCD Òý½Å
#define LCD_RST_SET     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET)
#define LCD_RST_CLR     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET)
#define LCD_RS_SET      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET)
#define LCD_RS_CLR      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET)
#define LCD_CS_SET      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)
#define LCD_CS_CLR      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)
#define LCD_LED_ON      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET)

// ´¥ÃþÒý½Å
#define TP_CS_SET       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET)
#define TP_CS_CLR       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET)

// TOUCH
#define TP_CS_SET       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET)
#define TP_CS_CLR       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET)
// ÆÁÄ»³ß´ç
#define LCD_W   320
#define LCD_H   240



uint8_t TP_Get_Calibrated(uint16_t *x, uint16_t *y);
void    TP_Init(void);
uint16_t TP_ReadX(void);
uint16_t TP_ReadY(void);

#endif