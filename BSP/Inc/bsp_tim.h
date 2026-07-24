/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:         bsp_delay.h
 Author: PAN        Version: V1.0       Date:2026/05/15
 Description:       系统延时封装（基于HAL_Delay）

 Function List:
     1. BSP_Delay_ms()     - 毫秒级延时
 History:
*******************************************************/

#ifndef __BSP_TIM_H
#define __BSP_TIM_H

#include "stdint.h"
 



void delay_us(uint32_t us);
void Sys_Delay(uint16_t time_ms);

#endif /* __BSP_TIM_H */