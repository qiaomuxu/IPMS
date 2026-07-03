#include "stm32f10x.h"
#include "FSR.h"
#include "delay.h"

void FSR_IO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_SetBits(GPIOC, GPIO_Pin_14);
}

void HW_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_SetBits(GPIOA, GPIO_Pin_15);
}

void SERVO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_SetBits(GPIOA, GPIO_Pin_11);
}

u8 FSR_Scan(u8 mode)
{
    static u8 key_up = 1;
    if(mode) key_up = 1;

    if(key_up && FSR_GPIO == 1)
    {
        delay_ms(10);
        key_up = 0;
        if(FSR_GPIO == 1) return KEY_PRESSED;
    }
    else if(FSR_GPIO == 0)
    {
        key_up = 1;
    }
    return KEY_RELEASED;
}

u8 HW_Scan(u8 mode)
{
    static u8 key_flag = 1;
    if(mode) key_flag = 1;

    if(key_flag && HW_GPIO == 0)
    {
        delay_ms(10);
        key_flag = 0;
        if(HW_GPIO == 0) return KEY_PRESSED;
    }
    else if(HW_GPIO == 1)
    {
        key_flag = 1;
    }
    return KEY_RELEASED;
}

void servo_ctrl(u8 angle)
{
    if(angle == 0)
    {
        for(int j = 0; j < 100; j++)
        {
            GPIO_SetBits(SERVO_PORT, SERVO_PIN);
            delay_ms(1);
            GPIO_ResetBits(SERVO_PORT, SERVO_PIN);
            delay_ms(19);
        }
    }
    else
    {
        for(int j = 0; j < 100; j++)
        {
            GPIO_SetBits(SERVO_PORT, SERVO_PIN);
            delay_ms(2);
            GPIO_ResetBits(SERVO_PORT, SERVO_PIN);
            delay_ms(18);
        }
    }
}
