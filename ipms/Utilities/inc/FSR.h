#ifndef __FSR_H
#define __FSR_H

#define FSR_GPIO   GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_14)
#define HW_GPIO    GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15)

#define SERVO_PIN  GPIO_Pin_11
#define SERVO_PORT GPIOA

#define KEY_PRESSED		1
#define KEY_RELEASED	0

void FSR_IO_Init(void);
void HW_GPIO_Init(void);
void SERVO_Init(void);
u8 HW_Scan(u8 mode);
u8 FSR_Scan(u8 mode);
void servo_ctrl(u8 angle);

#endif
