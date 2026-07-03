#include "stm32f10x.h"
#include "usart.h"
#include "stdio.h"

 

int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);   
    USART1->DR = (u8) ch;      
	return ch;
}

void USART1_init(void)
{
		GPIO_InitTypeDef   gpio_init_struct;
		USART_InitTypeDef  usart_init_struct;
		NVIC_InitTypeDef   NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_init_struct.GPIO_Pin = GPIO_Pin_9;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_init_struct);

    gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio_init_struct.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &gpio_init_struct);

    usart_init_struct.USART_BaudRate = 115200;
    usart_init_struct.USART_WordLength = USART_WordLength_8b;
    usart_init_struct.USART_StopBits = USART_StopBits_1;
    usart_init_struct.USART_Parity = USART_Parity_No;
    usart_init_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart_init_struct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &usart_init_struct);

    USART_Cmd(USART1, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}



void USART1_Send_Byte(u8 Data) 
{
	USART_SendData(USART1,Data);
	while( USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET );
}

void USART1_Send_String(u8 *Data) 
{
	while(*Data)
		USART1_Send_Byte(*Data++);
}





void USART1_SendChar(int32_t ch)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    {
    }
    USART_SendData(USART1, (uint8_t) ch);
}
u16 USART1_ReceiveChar(void)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
    {
    }
    return USART_ReceiveData(USART1);
}

void USART1_SendNum(u32 num)
{
		u8 ge,shi,bai,qian;
	
		ge=num%10;
		shi=num%100/10;
		bai=num%1000/100;
		qian=num/1000;
	
		USART1_SendChar(qian+0x30);
		USART1_SendChar(bai+0x30);
		USART1_SendChar(shi+0x30);
		USART1_SendChar(ge+0x30);
	
		USART1_SendChar(0x0d);
		USART1_SendChar(0x0a);
}
void USART1_SendNum_0d0a(u32 num)
{
		u8 ge,shi,bai,qian;
	
		ge=num%10;
		shi=num%100/10;
		bai=num%1000/100;
		qian=num/1000;
	
		USART1_SendChar(qian+0x30);
		USART1_SendChar(bai+0x30);
		USART1_SendChar(shi+0x30);
		USART1_SendChar(ge+0x30);
	
		USART1_SendChar(' ');

}



