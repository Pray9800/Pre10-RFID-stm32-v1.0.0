/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         bsp_usart.h
 Author: PAN        Version: V1.0       Date:2026/08/12
 Description:       485通讯

 Function List:
 History:
*******************************************************/

#ifndef __BSP_USART_H__
#define __BSP_USART_H__

#include "main.h"
#include "gpio.h"
#include "usart.h"
 
extern uint8_t UART2_Rxbuff[50];
extern uint8_t rx_temp; // 声明刚刚定义的单字节缓存
extern volatile uint8_t key_query_ready; //key查询标志位
void MCU_485_Receive_IT_Start( uint8_t *pData, uint16_t Size);
void MCU_485_Send(uint8_t *buf,uint8_t len);

#endif /* __BSP_USART_H__ */
