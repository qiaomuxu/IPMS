#ifndef __LTK_DEBUG_H
#define __LTK_DEBUG_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stdio.h"

#ifdef LTK_RELEASE
#define ltk_printf(format...)   ((void)0)
#define ltk_scanf(format...)    ((void)0)
#define ltk_getchar()           ((void)0)
#else
#define ltk_printf(format...)   printf(##format)
#define ltk_scanf(format...)    scanf(##format)
#define ltk_getchar()           getchar()
#endif

void assert_failed(uint8_t* file, uint32_t line);

#ifdef __cplusplus
}
#endif


#endif /* __LTK_DEBUG_H */
