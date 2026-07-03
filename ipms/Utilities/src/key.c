#include "stm32f10x.h"
#include "key.h"

void Key_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(KEY0_RCC_PERIPH, ENABLE);
    RCC_APB2PeriphClockCmd(KEY1_RCC_PERIPH, ENABLE);

    gpio_init_struct.GPIO_Pin = KEY0_GPIO_PIN;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY0_GPIO_PORT, &gpio_init_struct);

    gpio_init_struct.GPIO_Pin = KEY1_GPIO_PIN;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY1_GPIO_PORT, &gpio_init_struct);

    gpio_init_struct.GPIO_Pin = KEY2_PIN;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY1_PORT, &gpio_init_struct);
}

