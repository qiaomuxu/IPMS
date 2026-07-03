#include "stm32f10x.h"
#include "lcd.h"
#include "delay.h"
#include "board.h"
#include "font.h"
void Lcd_Gpio_Init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
		RCC_APB2PeriphClockCmd(LCD_CS_RCC | LCD_RD_RCC | LCD_WR_RCC | LCD_RS_RCC | LCD_REST_RCC |
                            LCD_DATA_RCC | RCC_APB2Periph_AFIO, ENABLE);
		GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
    gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init_struct.GPIO_Pin = LCD_CS_PIN;
    GPIO_Init(LCD_CS_PORT, &gpio_init_struct);
    
    gpio_init_struct.GPIO_Pin = LCD_RD_PIN;
    GPIO_Init(LCD_RD_PORT, &gpio_init_struct);

    gpio_init_struct.GPIO_Pin = LCD_WR_PIN;
    GPIO_Init(LCD_WR_PORT, &gpio_init_struct);

    gpio_init_struct.GPIO_Pin = LCD_RS_PIN;
    GPIO_Init(LCD_RS_PORT, &gpio_init_struct);
 
    gpio_init_struct.GPIO_Pin = LCD_REST_PIN;
    GPIO_Init(LCD_REST_PORT, &gpio_init_struct);
	
    gpio_init_struct.GPIO_Pin = LCD_DATA_PIN;
    GPIO_Init(LCD_DATA_PORT, &gpio_init_struct);
	
	GPIO_WriteBit(LCD_RD_PORT, LCD_RD_PIN,1);
	GPIO_WriteBit(LCD_WR_PORT, LCD_WR_PIN,1);
	GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN,1);
	GPIO_WriteBit(LCD_CS_PORT, LCD_CS_PIN,0);
}
void LCD_Writ_Bus(unsigned short bus_data)
{
	LCD_DATA_PORT->ODR = bus_data;
	GPIO_WriteBit(LCD_WR_PORT, LCD_WR_PIN,0);
	GPIO_WriteBit(LCD_WR_PORT, LCD_WR_PIN,1);
}
void LCD_Write_COM(u16 bus_data)	
{	
	GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN,0);
	LCD_Writ_Bus(bus_data);
}
void LCD_Write_DATA(u16 bus_data)	
{
	GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN,1);
	LCD_Writ_Bus(bus_data);
}
void LCD_Init(void)
{
	GPIO_WriteBit(LCD_REST_PORT, LCD_REST_PIN,1);
  delayms(10);	
	GPIO_WriteBit(LCD_REST_PORT, LCD_REST_PIN,0);
	delayms(10);
	GPIO_WriteBit(LCD_REST_PORT, LCD_REST_PIN,1);
	delayms(10);
	LCD_Write_COM(0xCF);  
	LCD_Write_DATA(0x00); 
	LCD_Write_DATA(0xC1); 
	LCD_Write_DATA(0X30); 
	LCD_Write_COM(0xED);  
	LCD_Write_DATA(0x64); 
	LCD_Write_DATA(0x03); 
	LCD_Write_DATA(0X12); 
	LCD_Write_DATA(0X81); 
	LCD_Write_COM(0xE8);  
	LCD_Write_DATA(0x85); 
	LCD_Write_DATA(0x10); 
	LCD_Write_DATA(0x7A); 
	LCD_Write_COM(0xCB);  
	LCD_Write_DATA(0x39); 
	LCD_Write_DATA(0x2C); 
	LCD_Write_DATA(0x00); 
	LCD_Write_DATA(0x34); 
	LCD_Write_DATA(0x02); 
	LCD_Write_COM(0xF7);  
	LCD_Write_DATA(0x20); 
	LCD_Write_COM(0xEA);  
	LCD_Write_DATA(0x00); 
	LCD_Write_DATA(0x00); 
	LCD_Write_COM(0xC0);    
	LCD_Write_DATA(0x1B);   
	LCD_Write_COM(0xC1);    
	LCD_Write_DATA(0x01);   
	LCD_Write_COM(0xC5);    
	LCD_Write_DATA(0x30); 	 
	LCD_Write_DATA(0x30); 	 
	LCD_Write_COM(0xC7);    
	LCD_Write_DATA(0XB7); 
	LCD_Write_COM(0x36);    
	LCD_Write_DATA(0x48); 
	LCD_Write_COM(0x3A);   
	LCD_Write_DATA(0x55); 
	LCD_Write_COM(0xB1);   
	LCD_Write_DATA(0x00);   
	LCD_Write_DATA(0x1A); 
	LCD_Write_COM(0xB6);    
	LCD_Write_DATA(0x0A); 
	LCD_Write_DATA(0xA2); 
	LCD_Write_COM(0xF2);    
	LCD_Write_DATA(0x00); 
	LCD_Write_COM(0x26);    
	LCD_Write_DATA(0x01); 
	LCD_Write_COM(0xE0);    
	LCD_Write_DATA(0x0F); 
	LCD_Write_DATA(0x2A); 
	LCD_Write_DATA(0x28); 
	LCD_Write_DATA(0x08); 
	LCD_Write_DATA(0x0E); 
	LCD_Write_DATA(0x08); 
	LCD_Write_DATA(0x54); 
	LCD_Write_DATA(0XA9); 
	LCD_Write_DATA(0x43); 
	LCD_Write_DATA(0x0A); 
	LCD_Write_DATA(0x0F); 
	LCD_Write_DATA(0x00); 
	LCD_Write_DATA(0x00); 
	LCD_Write_DATA(0x00); 
	LCD_Write_DATA(0x00); 		 
	LCD_Write_COM(0XE1);    
	LCD_Write_DATA(0x00); 
	LCD_Write_DATA(0x15); 
	LCD_Write_DATA(0x17); 
	LCD_Write_DATA(0x07); 
	LCD_Write_DATA(0x11); 
	LCD_Write_DATA(0x06); 
	LCD_Write_DATA(0x2B); 
	LCD_Write_DATA(0x56); 
	LCD_Write_DATA(0x3C); 
	LCD_Write_DATA(0x05); 
	LCD_Write_DATA(0x10); 
	LCD_Write_DATA(0x0F); 
	LCD_Write_DATA(0x3F); 
	LCD_Write_DATA(0x3F); 
	LCD_Write_DATA(0x0F); 
	LCD_Write_COM(0x2B); 
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x01);
	LCD_Write_DATA(0x3f);
	LCD_Write_COM(0x2A); 
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0xef);	 
	LCD_Write_COM(0x11);
	delay_ms(120);
	LCD_Write_COM(0x29);	
	LCD_Write_COM(0x36);
	LCD_Write_DATA(0x6C);
}

void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
{	
	LCD_Write_COM(0x2a);	
	LCD_Write_DATA(xStar>>8);
	LCD_Write_DATA(0x00FF&xStar);		
	LCD_Write_DATA(xEnd>>8);
	LCD_Write_DATA(0x00FF&xEnd);
	LCD_Write_COM(0x2b);	
	LCD_Write_DATA(yStar>>8);
	LCD_Write_DATA(0x00FF&yStar);		
	LCD_Write_DATA(yEnd>>8);
	LCD_Write_DATA(0x00FF&yEnd);	
	LCD_Write_COM(0x2C);				
}  
void LCD_Fill(unsigned short color)
{  	
	unsigned short i,j;			
	LCD_SetWindows(0,0,320,240);
	GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN,1);
	for(i=0;i<320;i++)
	{
		for(j=0;j<240;j++)
		LCD_Writ_Bus(color);	 
	}
}
void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_SetWindows(x,y,x,y);
	GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN,1);
	LCD_Writ_Bus(color); 	    
} 
void LCD_ShowChar(u16 x,u16 y, u8 num,u8 mode)
{  
	u8 temp;
	u8 pos,t;    
 	num=num-' ';
	if(!mode) 
	{
		for(pos=0;pos<18;pos++)
		{
			temp=asc2_1608[num][pos];		 
			for(t=0;t<8;t++)
			{                 
				if(temp&0x01)LCD_DrawPoint(x+t,y+pos,0xffff);
				else LCD_DrawPoint(x+t,y+pos,0x0000);
				temp>>=1; 
			}		
	}	
	}
	else
	{
		for(pos=0;pos<20;pos++)
		{
			temp=asc2_1608[num][pos];		 
			for(t=0;t<10;t++)
			{                 
				if(temp&0x01)LCD_DrawPoint(x+t,y+pos,0xffff);    
				temp>>=1; 
			}		
		}
	}
	LCD_SetWindows(0,0,320,240);    	   	 	  
} 
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}		
void LCD_ShowNumPoint(u16 x,u16 y,u16 num)
{
	LCD_ShowChar(x,y,num/10000+0x30,0);
	LCD_ShowChar(x+8*1,y,num/1000%10+0x30,0);
	LCD_ShowChar(x+8*2,y,num/100%10+0x30,0);
	LCD_ShowChar(x+8*3,y,'.',0);
	LCD_ShowChar(x+8*4,y,num/10%10+0x30,0);
	LCD_ShowChar(x+8*5,y,num%10+0x30,0);
}
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len)
{         	
	u8 t,temp;				   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
	 	LCD_ShowChar(x+t*8,y,temp+'0',0); 
	}
} 
u16 LCD_ReadPoint(u16 x,u16 y)
{
    u16 r,g,b;
    if(x>=320||y>=240)return 0;            
	LCD_SetWindows(x,y,x,y);
	LCD_Write_COM(0X2E);	
    GPIOB->CRL=0X88888888; 
    GPIOB->CRH=0X88888888; 
    GPIOB->ODR=0XFFFF;     

	GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN,1);
	GPIO_WriteBit(LCD_CS_PORT, LCD_CS_PIN,0);
    GPIO_WriteBit(LCD_RD_PORT, LCD_RD_PIN,0);
    delay_us(100);                    
	GPIO_WriteBit(LCD_RD_PORT, LCD_RD_PIN,1);
    GPIO_WriteBit(LCD_RD_PORT, LCD_RD_PIN,0);
    delay_us(100);                    
	r=GPIOB->IDR;	
	GPIO_WriteBit(LCD_RD_PORT, LCD_RD_PIN,1);

	GPIO_WriteBit(LCD_RD_PORT, LCD_RD_PIN,0);
	b=GPIOB->IDR;	
	GPIO_WriteBit(LCD_RD_PORT, LCD_RD_PIN,1);
        g=r&0XFF;
        g<<=8;
    GPIOB->CRL=0X33333333;       
    GPIOB->CRH=0X33333333;       
    GPIOB->ODR=0XFFFF;          
	return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));

}

void LCD_ShowString(u16 x, u16 y, const char *str)
{
    u16 i = 0;
    while(str[i] != '\0') {
        LCD_ShowChar(x + i * 8, y, str[i], 0);
        i++;
    }
}
