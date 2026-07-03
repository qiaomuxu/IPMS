#include "stm32f10x_it.h"
#include "stm32f10x.h"

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
  while (1)
  {
  }
}

void MemManage_Handler(void)
{
  while (1)
  {
  }
}

void BusFault_Handler(void)
{
  while (1)
  {
  }
}

void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

void DebugMon_Handler(void)
{
}

#if (!defined LTK_FREERTOS && !defined LTK_UCOS)
void SysTick_Handler(void)
{
}
#endif

void WWDG_IRQHandler(void)
{
}

void PVD_IRQHandler(void)
{
}

void TAMPER_IRQHandler(void)
{
}

void RTC_IRQHandler(void)
{
}

void FLASH_IRQHandler(void)
{
}

void RCC_IRQHandler(void)
{
}

void exti_handler(uint32_t exti_line)
{
    #ifdef LTK_KEY_EXTI
    if(EXTI_GetITStatus(exti_line) != RESET)
    {
        ltk_led_toggle(LTK_LED0);
        EXTI_ClearITPendingBit(exti_line);
    }
    #endif
    
    #ifdef LTK_IWDG
    if(EXTI_GetITStatus(exti_line) != RESET)
    {
        IWDG_ReloadCounter();
        EXTI_ClearITPendingBit(exti_line);
    }
    #endif
    
    #ifdef LTK_TOUCHSCREEN
    pen_state_t *pen_st;
    if(EXTI_GetITStatus(exti_line) != RESET)
    {       
        pen_st = ltk_get_penstate();
        pen_st->force_adjust = 1;
        EXTI_ClearITPendingBit(exti_line);
    }
    #endif
}

void EXTI0_IRQHandler(void)
{
}

void EXTI1_IRQHandler(void)
{
}

void EXTI2_IRQHandler(void)
{
    exti_handler(EXTI_Line2);
}

void EXTI3_IRQHandler(void)
{
    exti_handler(EXTI_Line3);
}

void EXTI4_IRQHandler(void)
{
}

void DMA1_Channel1_IRQHandler(void)
{
}

void DMA1_Channel2_IRQHandler(void)
{
}

void DMA1_Channel3_IRQHandler(void)
{
}

void DMA1_Channel4_IRQHandler(void)
{
}

void DMA1_Channel5_IRQHandler(void)
{
}

void DMA1_Channel6_IRQHandler(void)
{
}

void DMA1_Channel7_IRQHandler(void)
{
}


void ADC1_2_IRQHandler(void)
{
    #ifdef LTK_ADC
    set_adc_value(ADC_GetConversionValue(ADC_NUM));
    #endif
}

void USB_HP_CAN1_TX_IRQHandler(void)
{
    while(1)
    {
    }
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    #ifdef LTK_USB
    USB_Istr();
    #endif
}

void CAN_RX1_IRQHandler(void)
{
}

void CAN_SCE_IRQHandler(void)
{
}

void EXTI9_5_IRQHandler(void)
{
    #ifdef LTK_TOUCHSCREEN
    pen_state_t *pen_st;
    #ifdef LTK_FREERTOS
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    #endif
    if(EXTI_GetITStatus(TS_PEN_EXTI_LINE) != RESET)
    {
        EXTI_ClearITPendingBit(TS_PEN_EXTI_LINE);
        pen_st = ltk_get_penstate();
        pen_st->pen_pressed = KEY_DOWN;
        #ifdef LTK_FREERTOS
        xSemaphoreGiveFromISR(binary_sem_ts, &xHigherPriorityTaskWoken);
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
        #endif
    }
    #endif
}

void TIM1_BRK_IRQHandler(void)
{
}

void TIM1_UP_IRQHandler(void)
{
}

void TIM1_TRG_COM_IRQHandler(void)
{
}

void TIM1_CC_IRQHandler(void)
{
}

void TIM2_IRQHandler(void)
{
}

void TIM3_IRQHandler(void)
{
}

void TIM4_IRQHandler(void)
{
}

void I2C1_EV_IRQHandler(void)
{
}

void I2C1_ER_IRQHandler(void)
{
}

void I2C2_EV_IRQHandler(void)
{
}

void I2C2_ER_IRQHandler(void)
{
}

void SPI1_IRQHandler(void)
{
}

void SPI2_IRQHandler(void)
{
}

void USART1_IRQHandler(void)
{
}

void USART2_IRQHandler(void)
{
}

void USART3_IRQHandler(void)
{
}

void EXTI15_10_IRQHandler(void)
{
}

void RTCAlarm_IRQHandler(void)
{
}

void USBWakeUp_IRQHandler(void)
{
    #ifdef LTK_USB
    EXTI_ClearITPendingBit(EXTI_Line18);
    #endif
}

void TIM8_BRK_IRQHandler(void)
{
}

void TIM8_UP_IRQHandler(void)
{
}

void TIM8_TRG_COM_IRQHandler(void)
{
}

void TIM8_CC_IRQHandler(void)
{
}

void ADC3_IRQHandler(void)
{
}

void FSMC_IRQHandler(void)
{
}

void SDIO_IRQHandler(void)
{
}

void TIM5_IRQHandler(void)
{
}

void SPI3_IRQHandler(void)
{
}

void UART4_IRQHandler(void)
{
}

void UART5_IRQHandler(void)
{
}

void TIM6_IRQHandler(void)
{
}

void TIM7_IRQHandler(void)
{
}

void DMA2_Channel1_IRQHandler(void)
{
}

void DMA2_Channel2_IRQHandler(void)
{
}

void DMA2_Channel3_IRQHandler(void)
{
}

void DMA2_Channel4_5_IRQHandler(void)
{
}
