#include "stm32f10x.h"
#include "usart.h"
#include "board.h"
#include "serial_image.h"

#define SERIAL_IMG_MAGIC0 0xA5
#define SERIAL_IMG_MAGIC1 0x5A
#define SERIAL_IMG_MAGIC2 0x5A
#define SERIAL_IMG_MAGIC3 0xA5

static void send_u16_le(uint16_t v)
{
    USART1_SendChar((int32_t)(v & 0xFFU));
    USART1_SendChar((int32_t)((v >> 8) & 0xFFU));
}

static void send_u32_le(uint32_t v)
{
    USART1_SendChar((int32_t)(v & 0xFFU));
    USART1_SendChar((int32_t)((v >> 8) & 0xFFU));
    USART1_SendChar((int32_t)((v >> 16) & 0xFFU));
    USART1_SendChar((int32_t)((v >> 24) & 0xFFU));
}

void send_pic_using_USART(void)
{
    vu16 a, b;
    vu16 AA = 0, BB = 0;
    vu16 color;

    while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0) == 1);
    GPIO_WriteBit(FIFO_WRST_PORT, FIFO_WRST_PIN, 0);
    GPIO_WriteBit(FIFO_WRST_PORT, FIFO_WRST_PIN, 1);
    GPIO_WriteBit(FIFO_WR_PORT, FIFO_WR_PIN, 1);
    while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0) == 0);
    while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0) == 1);
    GPIO_WriteBit(FIFO_WR_PORT, FIFO_WR_PIN, 0);

    FIFO_Reset_Read_Addr();

    for (a = 0; a < 240; a++)
    {
        for (b = 0; b < 320; b++)
        {
            GPIOC->BRR = 1 << 4;
            AA = GPIOA->IDR;
            GPIOC->BSRR = 1 << 4;

            GPIOC->BRR = 1 << 4;
            BB = GPIOA->IDR & 0x00ffU;
            GPIOC->BSRR = 1 << 4;

            color = (AA << 8) | BB;
            USART1_SendChar((int32_t)((color >> 8) & 0xFFU));
            USART1_SendChar((int32_t)(color & 0xFFU));
        }
    }
}
