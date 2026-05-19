#include "spi.h"
#include "touch.h"
#include "stdio.h"
#include "usart.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
extern SPI_HandleTypeDef hspi1;  // “—∏ƒ SPI1


 uint16_t pressure;
 

 
static uint8_t TP_ReadWrite(uint8_t data)
{
    uint8_t rx;
    HAL_SPI_TransmitReceive(&hspi1, &data, &rx, 1, 100);
    return rx;
}

static uint16_t TP_Read_AD(uint8_t cmd)
{
    uint16_t temp = 0;
	LCD_CS_SET;
    TP_CS_CLR;
    TP_ReadWrite(cmd);
    temp = TP_ReadWrite(0xFF) << 8;
    temp |= TP_ReadWrite(0xFF);
    TP_CS_SET;
    temp >>= 4;
    return temp;
}

uint16_t TP_ReadX(void)
{
    return TP_Read_AD(0xD0);
}

uint16_t TP_ReadY(void)
{
    return TP_Read_AD(0x90);
}

// ∂¡»°—π¡¶÷µ£®Z1 ∫Õ Z2£©
static uint16_t TP_ReadPressure(void)
{
    uint16_t z1 = TP_Read_AD(0xB1);
    uint16_t z2 = TP_Read_AD(0xC1);
    if (z1 == 0) return 0;
    return (z2 * 100) / z1;   // ºÚµ•—π¡¶÷µ
}

void TP_Init(void)
{
    TP_CS_SET;
   HAL_Delay(10);  // ‚úÖ ÂøÖÈ°ªÁî®Ëøô‰∏™
    TP_ReadX();  // Œ»∂®
}

 // Êñ∞Â¢ûÔºöÊ£ÄÊµãËß¶Êë∏ÊòØÂê¶Êåâ‰∏ã
static uint8_t TP_IsPressed(void)
{
    uint16_t z1 = TP_Read_AD(0xB1);
    uint16_t z2 = TP_Read_AD(0xC1);
	char text[20];
		sprintf(text,"z1=%d z2=%d\r\n",z1,z2);
	HAL_UART_Transmit(&huart2,(uint8_t *)text,strlen(text),30);
    
    // Â∏∏ËßÅËß¶Êë∏ÈòàÂÄºÔºöZ1 < 1000 Êàñ Z2 < 1000 Âç≥ËÆ§‰∏∫Êåâ‰∏ãÔºàÂèØÂæÆË∞ÉÔºâ
    if (z1 >50 && z2 < 3000)  // ÈòàÂÄºÊ†πÊçÆÂÆûÈôÖËß¶Êë∏Ë∞ÉËØï
        return 1;
    else
        return 0;
}

uint8_t TP_Get_Calibrated(uint16_t *x, uint16_t *y)
{
    // ========== 1. —π¡¶ºÏ≤‚£®¬À≥˝Œﬁ¥•√˛‘Î…˘£© ==========
    // 1. ÂÖàÂà§Êñ≠ÊòØÂê¶Êåâ‰∏ã
    if (!TP_IsPressed())
        return 0;

    // 2. ËØªÂèñÂùêÊ†á
    uint16_t raw_x = TP_ReadX();
    uint16_t raw_y = TP_ReadY();

    // ========== 3. ∑∂Œß¬À≤®£®πÿ±’£¨±‹√‚ŒÛ≈–£© ==========
    // if (raw_x < 200 || raw_x > 4095 || raw_y < 200 || raw_y > 4095)
    //     return 0;

    // ========== 4. ”≥…‰£® π”√ƒ„“—æ≠—È÷§’˝»∑µƒ∫Í≈‰÷√£© ==========
    int16_t lcd_x, lcd_y;

    // ? ’˝»∑  ≈‰ 2.8¥Á∆¡
    #define TOUCH_SWAP_XY    1
    #define TOUCH_MIRROR_X   1
    #define TOUCH_MIRROR_Y   1




    #define TX_MIN   170
    #define TX_MAX   1900
    #define TY_MIN   190
    #define TY_MAX   1900

#if TOUCH_SWAP_XY == 0
    lcd_x = (raw_x - TX_MIN) * (LCD_W - 1) / (TX_MAX - TX_MIN);
    lcd_y = (raw_y - TY_MIN) * (LCD_H - 1) / (TY_MAX - TY_MIN);
#else
    lcd_x = (raw_y - TY_MIN) * (LCD_W - 1) / (TY_MAX - TY_MIN);
    lcd_y = (raw_x - TX_MIN) * (LCD_H - 1) / (TX_MAX - TX_MIN);
#endif

    if (TOUCH_MIRROR_X) lcd_x = (LCD_W - 1) - lcd_x;
    if (TOUCH_MIRROR_Y) lcd_y = (LCD_H - 1) - lcd_y;

    if (lcd_x < 0) lcd_x = 0;
    if (lcd_x >= LCD_W) lcd_x = LCD_W - 1;
    if (lcd_y < 0) lcd_y = 0;
    if (lcd_y >= LCD_H) lcd_y = LCD_H - 1;

    *x = (uint16_t)lcd_x;
    *y = (uint16_t)lcd_y;
    return 1;
}