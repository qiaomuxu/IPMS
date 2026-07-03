#include "bsp_esp8266.h"
#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "delay.h"

static void ESP8266_GPIO_Config(void);
static void ESP8266_USART_Config(void);
static void ESP8266_USART_NVIC_Configuration(void);

volatile uint8_t ucTcpClosedFlag = 0;

struct  STRUCT_USARTx_Fram strEsp8266_Fram_Record = { 0 };

void ESP8266_Init(void)
{
	ESP8266_USART_Config();
}

void ESP8266_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	macESP8266_CH_PD_APBxClock_FUN(macESP8266_CH_PD_CLK, ENABLE);

	GPIO_InitStructure.GPIO_Pin = macESP8266_CH_PD_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(macESP8266_CH_PD_PORT, &GPIO_InitStructure);

	macESP8266_RST_APBxClock_FUN(macESP8266_RST_CLK, ENABLE);

	GPIO_InitStructure.GPIO_Pin = macESP8266_RST_PIN;
	GPIO_Init(macESP8266_RST_PORT, &GPIO_InitStructure);
}

static void ESP8266_USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	macESP8266_USART_APBxClock_FUN(macESP8266_USART_CLK, ENABLE);
	macESP8266_USART_GPIO_APBxClock_FUN(macESP8266_USART_GPIO_CLK, ENABLE);

	GPIO_InitStructure.GPIO_Pin =  macESP8266_USART_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(macESP8266_USART_TX_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = macESP8266_USART_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(macESP8266_USART_RX_PORT, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = macESP8266_USART_BAUD_RATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(macESP8266_USARTx, &USART_InitStructure);

	USART_ITConfig(macESP8266_USARTx, USART_IT_RXNE, ENABLE);
	USART_ITConfig(macESP8266_USARTx, USART_IT_IDLE, ENABLE);

	ESP8266_USART_NVIC_Configuration();

	USART_Cmd(macESP8266_USARTx, ENABLE);
}

static void ESP8266_USART_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(macNVIC_PriorityGroup_x);

	NVIC_InitStructure.NVIC_IRQChannel = macESP8266_USART_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void ESP8266_Rst(void)
{
	#if 0
	 ESP8266_Cmd("AT+RST", "OK", "ready", 2500);
	#else
	 macESP8266_RST_LOW_LEVEL();
	 delay_ms(500);
	 macESP8266_RST_HIGH_LEVEL();
	#endif
}

bool ESP8266_Cmd(char * cmd, char * reply1, char * reply2, u32 waittime)
{
	strEsp8266_Fram_Record.InfBit.FramLength = 0;

	macESP8266_Usart("%s\r\n", cmd);

	if ((reply1 == 0) && (reply2 == 0))
		return true;

	delay_ms(waittime);

	strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';

	if ((reply1 != 0) && (reply2 != 0))
		return ((bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply1) ||
					 (bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply2));

	else if (reply1 != 0)
		return ((bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply1));

	else
		return ((bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply2));
}

bool ESP8266_ByteData(uint8_t  * ByteData, char * reply1, char * reply2, uint32_t waittime)
{
  uint8_t len=0,i=0;

	strEsp8266_Fram_Record.InfBit.FramLength = 0;
	len=strlen((const char*)ByteData);

	for(i=0;i<len;i++)
	{
	 USART_SendData(macESP8266_USARTx,(u8)ByteData);
   while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
	}
	if ((reply1 == 0) && (reply2 == 0))
		return true;

	delay_ms(waittime);

	strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';

	macPC_Usart("%s", strEsp8266_Fram_Record.Data_RX_BUF);
	if ((reply1 != 0) && (reply2 != 0))
		return ((bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply1) ||
					 (bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply2));

	else if (reply1 != 0)
		return ((bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply1));

	else
		return ((bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply2));
}

void ESP8266_AT_Test(void)
{
	char count=0;

	delay_ms(1000);
	while (count < 10)
	{
		if(ESP8266_Cmd("AT", "OK", NULL, 500)) return;
		++count;
	}
}

bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode)
{
	switch (enumMode)
	{
		case STA:
			return ESP8266_Cmd("AT+CWMODE=1", "OK", "no change", 2500);

	  case AP:
		  return ESP8266_Cmd("AT+CWMODE=2", "OK", "no change", 2500);

		case STA_AP:
		  return ESP8266_Cmd("AT+CWMODE=3", "OK", "no change", 2500);

	  default:
		  return false;
	}
}

bool ESP8266_JoinAP(char * pSSID, char * pPassWord)
{
	char cCmd[120];

	sprintf(cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord);

	return ESP8266_Cmd(cCmd, "OK", NULL, 5000);
}

bool ESP8266_BuildAP(char * pSSID, char * pPassWord, ENUM_AP_PsdMode_TypeDef enunPsdMode)
{
	char cCmd[120];

	sprintf(cCmd, "AT+CWSAP=\"%s\",\"%s\",1,%d", pSSID, pPassWord, enunPsdMode);

	return ESP8266_Cmd(cCmd, "OK", 0, 1000);
}

bool ESP8266_Enable_MultipleId(FunctionalState enumEnUnvarnishTx)
{
	char cStr[20];

	sprintf(cStr, "AT+CIPMUX=%d", (enumEnUnvarnishTx ? 1 : 0));

	return ESP8266_Cmd(cStr, "OK", 0, 500);
}

bool ESP8266_Link_Server(ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id)
{
	char cStr[100] = { 0 }, cCmd[120];

  switch (enumE)
  {
		case enumTCP:
		  sprintf(cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum);
		  break;

		case enumUDP:
		  sprintf(cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum);
		  break;

		default:
			break;
  }

  if (id < 5)
    sprintf(cCmd, "AT+CIPSTART=%d,%s", id, cStr);

  else
	  sprintf(cCmd, "AT+CIPSTART=%s", cStr);

	return ESP8266_Cmd(cCmd, "OK", "ALREAY CONNECT", 4000);
}

bool ESP8266_StartOrShutServer(FunctionalState enumMode, char * pPortNum, char * pTimeOver)
{
	char cCmd1[120], cCmd2[120];

	if (enumMode)
	{
		sprintf(cCmd1, "AT+CIPSERVER=%d,%s", 1, pPortNum);
		sprintf(cCmd2, "AT+CIPSTO=%s", pTimeOver);

		return (ESP8266_Cmd(cCmd1, "OK", 0, 500) &&
						ESP8266_Cmd(cCmd2, "OK", 0, 500));
	}

	else
	{
		sprintf(cCmd1, "AT+CIPSERVER=%d,%s", 0, pPortNum);

		return ESP8266_Cmd(cCmd1, "OK", 0, 500);
	}
}

uint8_t ESP8266_Get_LinkStatus(void)
{
	if (ESP8266_Cmd("AT+CIPSTATUS", "OK", 0, 500))
	{
		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "STATUS:2\r\n"))
			return 2;

		else if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "STATUS:3\r\n"))
			return 3;

		else if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "STATUS:4\r\n"))
			return 4;
	}

	return 0;
}

uint8_t ESP8266_Get_IdLinkStatus(void)
{
	uint8_t ucIdLinkStatus = 0x00;

	if (ESP8266_Cmd("AT+CIPSTATUS", "OK", 0, 500))
	{
		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:0,"))
			ucIdLinkStatus |= 0x01;
		else
			ucIdLinkStatus &= ~0x01;

		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:1,"))
			ucIdLinkStatus |= 0x02;
		else
			ucIdLinkStatus &= ~0x02;

		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:2,"))
			ucIdLinkStatus |= 0x04;
		else
			ucIdLinkStatus &= ~0x04;

		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:3,"))
			ucIdLinkStatus |= 0x08;
		else
			ucIdLinkStatus &= ~0x08;

		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:4,"))
			ucIdLinkStatus |= 0x10;
		else
			ucIdLinkStatus &= ~0x10;
	}

	return ucIdLinkStatus;
}

uint8_t ESP8266_Inquire_ApIp(char * pApIp, uint8_t ucArrayLength)
{
	char uc;
	char * pCh;

  ESP8266_Cmd("AT+CIFSR", "OK", 0, 500);

	pCh = strstr(strEsp8266_Fram_Record.Data_RX_BUF, "APIP,\"");

	if (pCh)
		pCh += 6;

	else
		return 0;

	for (uc = 0; uc < ucArrayLength; uc++)
	{
		pApIp[uc] = *(pCh + uc);

		if (pApIp[uc] == '\"')
		{
			pApIp[uc] = '\0';
			break;
		}
	}

	return 1;
}

bool ESP8266_UnvarnishSend(void)
{
	if (!ESP8266_Cmd("AT+CIPMODE=1", "OK", 0, 500))
		return false;

	return ESP8266_Cmd("AT+CIPSEND", "OK", ">", 500);
}

void ESP8266_ExitUnvarnishSend(void)
{
	delay_ms(1000);

	macESP8266_Usart("+++");

	delay_ms(500);
}

bool ESP8266_SendString(FunctionalState enumEnUnvarnishTx, char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId)
{
	char cStr[20];
	bool bRet = false;

	if (enumEnUnvarnishTx)
	{
		macESP8266_Usart("%s", pStr);
		bRet = true;
	}
	else
	{
		if (ucId < 5)
			sprintf(cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2);

		else
			sprintf(cStr, "AT+CIPSEND=%d", ulStrLength);

		ESP8266_Cmd(cStr, "> ", 0, 1000);

		bRet = ESP8266_Cmd(pStr, "SEND OK", 0, 1000);
  }

	return bRet;
}

bool ESP8266_SendByte(FunctionalState enumEnUnvarnishTx,u8 *byte, uint32_t ulbyteLength, ENUM_ID_NO_TypeDef ucId)
{
	char cStr[20];
	bool bRet = false;
	unsigned char i=0;

	if (enumEnUnvarnishTx)
	{
	  for(i=0;i<ulbyteLength;i++)
		{
		USART_SendData(macESP8266_USARTx,byte[i]);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
		}
		bRet = true;
	}

	else
	{
		if (ucId < 5)
			sprintf(cStr, "AT+CIPSEND=%d,%d", ucId, ulbyteLength + 2);

		else
			sprintf(cStr, "AT+CIPSEND=%d", ulbyteLength + 2);

		ESP8266_Cmd(cStr, "> ", 0, 1000);

		bRet = ESP8266_ByteData(byte, "SEND OK", 0, 1000);
  }

	return bRet;
}

char * ESP8266_ReceiveString(FunctionalState enumEnUnvarnishTx)
{
	char * pRecStr = 0;

	strEsp8266_Fram_Record.InfBit.FramLength = 0;
	strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;

	while (!strEsp8266_Fram_Record.InfBit.FramFinishFlag);
	strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';

	if (enumEnUnvarnishTx)
		pRecStr = strEsp8266_Fram_Record.Data_RX_BUF;

	else
	{
		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+IPD"))
			pRecStr = strEsp8266_Fram_Record.Data_RX_BUF;
	}

	return pRecStr;
}

void macESP8266_USART_INT_FUN(void)
{
	uint8_t ucCh;

	if (USART_GetITStatus(macESP8266_USARTx, USART_IT_RXNE) != RESET)
	{
		ucCh  = USART_ReceiveData(macESP8266_USARTx);

		if (strEsp8266_Fram_Record.InfBit.FramLength < (RX_BUF_MAX_LEN - 1))
			strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength++] = ucCh;
	}

	if (USART_GetITStatus(macESP8266_USARTx, USART_IT_IDLE) == SET)
	{
    strEsp8266_Fram_Record.InfBit.FramFinishFlag = 1;

		ucCh = USART_ReceiveData(macESP8266_USARTx);

		ucTcpClosedFlag = strstr(strEsp8266_Fram_Record.Data_RX_BUF, "CLOSED\r\n") ? 1 : 0;
  }
}
