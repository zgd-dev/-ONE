#include "lcd.h"
#include "spi.h"
#include "touch.h"
#include "stdlib.h"

extern SPI_HandleTypeDef hspi1;

static void LCD_SPI_Write(uint8_t data)
{
    HAL_SPI_Transmit(&hspi1, &data, 1, 100);
}

void LCD_WR_REG(uint8_t cmd)
{
    LCD_CS_CLR;
    LCD_RS_CLR;
    LCD_SPI_Write(cmd);
    LCD_CS_SET;
}

void LCD_WR_DATA(uint8_t data)
{
    LCD_CS_CLR;
    LCD_RS_SET;
    LCD_SPI_Write(data);
    LCD_CS_SET;
}

void LCD_SetWindows(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(x1>>8);
    LCD_WR_DATA(x1);
    LCD_WR_DATA(x2>>8);
    LCD_WR_DATA(x2);

    LCD_WR_REG(0x2B);
    LCD_WR_DATA(y1>>8);
    LCD_WR_DATA(y1);
    LCD_WR_DATA(y2>>8);
    LCD_WR_DATA(y2);

    LCD_WR_REG(0x2C);
}

void LCD_Init(void)
{
    LCD_RST_CLR;
    HAL_Delay(100);
    LCD_RST_SET;
    HAL_Delay(100);

    LCD_WR_REG(0xCF);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xC1);
    LCD_WR_DATA(0X30);
    LCD_WR_REG(0xED);
    LCD_WR_DATA(0x64);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0X12);
    LCD_WR_DATA(0X81);
    LCD_WR_REG(0xE8);
    LCD_WR_DATA(0x85);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x7A);
    LCD_WR_REG(0xCB);
    LCD_WR_DATA(0x39);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x34);
    LCD_WR_DATA(0x02);
    LCD_WR_REG(0xF7);
    LCD_WR_DATA(0x20);
    LCD_WR_REG(0xEA);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xC0);
    LCD_WR_DATA(0x10);
    LCD_WR_REG(0xC1);
    LCD_WR_DATA(0x10);
    LCD_WR_REG(0xC5);
    LCD_WR_DATA(0x45);
    LCD_WR_DATA(0x15);
    LCD_WR_REG(0xC7);
    LCD_WR_DATA(0x90);

    LCD_WR_REG(0x36);
    LCD_WR_DATA(0x68);


// ∫·∆¡ 320x240 ±ÿ–Îº”’‚¡Ω––£°
LCD_WR_REG(0x2A);	// …Ë÷√¡–µÿ÷∑
LCD_WR_DATA(0x00);	// ∆ ºX = 0
LCD_WR_DATA(0x00);
LCD_WR_DATA(0x01);	// Ω· ¯X = 320-1
LCD_WR_DATA(0x3F);

LCD_WR_REG(0x2B);	// …Ë÷√––µÿ÷∑
LCD_WR_DATA(0x00);	// ∆ ºY = 0
LCD_WR_DATA(0x00);
LCD_WR_DATA(0x00);	// Ω· ¯Y = 240-1
LCD_WR_DATA(0xEF);
    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x55);

    LCD_WR_REG(0xB1);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x1A);

    LCD_WR_REG(0xB6);
    LCD_WR_DATA(0x0A);
    LCD_WR_DATA(0xA2);

    LCD_WR_REG(0xF2);
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0x26);
    LCD_WR_DATA(0x01);

    LCD_WR_REG(0xE0);
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x29);
    LCD_WR_DATA(0x24);
    LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x4E);
    LCD_WR_DATA(0x78);
    LCD_WR_DATA(0x3C);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x14);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x16);
    LCD_WR_DATA(0x1B);
    LCD_WR_DATA(0x04);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x31);
    LCD_WR_DATA(0x33);
    LCD_WR_DATA(0x42);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x0B);
    LCD_WR_DATA(0x28);
    LCD_WR_DATA(0x2F);
    LCD_WR_DATA(0x0F);

    LCD_WR_REG(0x11);
    HAL_Delay(120);
    LCD_WR_REG(0x29);

    LCD_LED_ON;
}

void LCD_WritePixels(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *color_p, uint32_t pixel_count)
{
    LCD_SetWindows(x1, y1, x2, y2);
    LCD_CS_CLR;
    LCD_RS_SET;
    
    // ‰∏ÄÊ¨°ÊÄßÈÄöËøá SPI ÂèëÈÄÅÊâÄÊúâÂÉèÁ¥†Êï∞ÊçÆ
    // Ê≥®ÊÑèÔºöHAL_SPI_Transmit ÂèØ‰ª•ÂèëÈÄÅ‰ªªÊÑèÈïøÂ∫¶ÁöÑÊï∞ÊçÆ
    HAL_SPI_Transmit(&hspi1, (uint8_t *)color_p, pixel_count * 2, HAL_MAX_DELAY);
    
    LCD_CS_SET;
}

void LCD_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint32_t total = (x2 - x1 + 1) * (y2 - y1 + 1);
    LCD_SetWindows(x1, y1, x2, y2);
    LCD_CS_CLR;
    LCD_RS_SET;
    for (uint32_t i = 0; i < total; i++) {
        HAL_SPI_Transmit(&hspi1, (uint8_t*)&color, 2, 100);
    }
    LCD_CS_SET;
}

//    ?     ?   Bresenham  ?
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    int16_t dx = abs(x2 - x1);
    int16_t dy = abs(y2 - y1);
    int16_t sx = x1 < x2 ? 1 : -1;
    int16_t sy = y1 < y2 ? 1 : -1;
    int16_t err = dx - dy;
    int16_t e2;

    while (1)
    {
        LCD_DrawPoint(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

//   ?   ??     3x3 ? ?  ÔÇ
void LCD_DrawBigPoint(uint16_t x, uint16_t y, uint16_t color)
{
    int16_t x1 = x - 1;
    int16_t y1 = y - 1;
    int16_t x2 = x + 1;
    int16_t y2 = y + 1;
    //  ? ®π 
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= LCD_W) x2 = LCD_W - 1;
    if (y2 >= LCD_H) y2 = LCD_H - 1;
    LCD_Fill(x1, y1, x2, y2, color);
}

void LCD_Clear(uint16_t color)
{
    LCD_SetWindows(0,0,LCD_W-1,LCD_H-1);
    uint8_t h = color>>8;
    uint8_t l = color&0xFF;
    LCD_CS_CLR;
    LCD_RS_SET;
    for(int i=0;i<LCD_W*LCD_H;i++){
        LCD_SPI_Write(h);
        LCD_SPI_Write(l);
    }
    LCD_CS_SET;
}

void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
    LCD_SetWindows(x,y,x,y);
    LCD_CS_CLR;
    LCD_RS_SET;
    LCD_SPI_Write(color >> 8);
    LCD_SPI_Write(color & 0xFF);
    LCD_CS_SET;
}

//       
void LCD_DrawRectangle(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
    uint16_t x,y;
    for(x=x1;x<=x2;x++){
        for(y=y1;y<=y2;y++){
            LCD_DrawPoint(x,y,color);
        }
    }
}

//   ? ? 
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint16_t color)
{
    uint8_t temp,pos,t;
    num=num-' ';
    for(pos=0;pos<16;pos++)
    {
        //       ?  ASCII  ?      ?  ?   ?   
        temp=0x01;
        for(t=0;t<8;t++)
        {
            if(temp&0x01)
                LCD_DrawPoint(x+t,y+pos,color);
            temp>>=1;
        }
    }
}

//   ? ?   
void LCD_ShowString(uint16_t x,uint16_t y,char *str,uint16_t color)
{
    while(*str)
    {
        LCD_ShowChar(x,y,*str,color);
        x+=8;
        if(x>LCD_W-8)
        {
            x=0;
            y+=16;
        }
        str++;
    }
}