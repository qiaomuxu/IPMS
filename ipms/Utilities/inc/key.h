#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
 extern "C" {
#endif 
	 
#define KEY1  GPIO_ReadInputDataBit(KEY1_PORT,KEY1_PIN)
#define KEY2  GPIO_ReadInputDataBit(KEY1_PORT,KEY2_PIN)	 
#define KEY1_PRES	1
#define KEY2_PRES	2
	 
void Key_init(void);
u8 KEY_Scan(u8);   
	 
#ifdef __cplusplus
}
#endif


#endif /* __LTK_KEY_H */
