#ifndef __LCD_H
#define __LCD_H
	 
#include "stm32f10x.h"
#include "lcd.h"
	 
void Lcd_Gpio_Init(void);
void LCD_Writ_Bus(u16 bus_data);
void LCD_Write_COM(u16 bus_data);
void LCD_Write_DATA(u16 bus_data);
void LCD_Fast_clear(u16 color);
void LCD_Init(void);
void Pant(int dcolor);
u16 LCD_ReadPoint(u16 x,u16 y);
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd);
void LCD_Fill(unsigned short color);
void LCD_DrawPoint(u16 x,u16 y,u16 color);
void LCD_ShowChar(u16 x,u16 y, u8 num,u8 mode);
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len);
void LCD_ShowNumPoint(u16 x,u16 y,u16 num);
void LCD_ShowString(u16 x, u16 y, const char *str);
#ifdef __cplusplus
}
#endif


#endif /* __LTK_GPIO_H */
