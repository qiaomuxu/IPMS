#ifndef __LTK_SYSTICK_H
#define __LTK_SYSTICK_H

#ifdef __cplusplus
 extern "C" {
#endif 

#ifdef LTK_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

#ifdef LTK_UCOS
#include "os.h"
#endif

#if defined LTK_FREERTOS
#define ltk_systick_delay vTaskDelay
#elif defined LTK_UCOS
#define ltk_systick_delay(systick_delay)\
{\
    OS_ERR err;\
    OSTimeDly(systick_delay, OS_OPT_TIME_DLY, &err);\
}
#else
void ltk_systick_delay(uint32_t time);
uint32_t ltk_get_systick_timer(void);
void ltk_increase_systick_timer(void);
#endif

void ltk_systick_init(uint32_t systick_hz);

#ifdef __cplusplus
}
#endif


#endif /* __LTK_SYSTICK_H */
