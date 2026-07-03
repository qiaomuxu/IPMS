#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
 extern "C" {
#endif 

void USART1_init(void);
void USART1_SendChar(int32_t ch);
void USART1_SendNum(u32 num);
u16  USART1_ReceiveChar(void);
void USART1_Send_Byte(u8 Data);
void USART1_Send_String(u8 *Data);


#ifdef USART1_IRQ
void ltk_usart_nvic_init(void);
#endif

#ifdef __cplusplus
}
#endif


#endif /* __LTK_USART_H */
