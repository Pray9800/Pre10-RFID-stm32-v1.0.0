/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:       bsp_i2c.h
 Author: PAN      Version: V1.0      Date: 2026/07/20
 Description:     基于 I2C2 的 RC522 读卡器底层 BSP 驱动
*******************************************************/

#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include "main.h"
#include "i2c.h"

#define BSP_I2C_TIMEOUT        100    //100ms的等待时间

/* API 接口导出 */

uint8_t BSP_I2C_IsDeviceReady(uint16_t device_addr);
HAL_StatusTypeDef BSP_I2C_WriteReg(uint16_t device_addr, uint8_t reg_addr, uint8_t value);
uint8_t BSP_I2C_ReadReg(uint16_t device_addr, uint8_t reg_addr);
HAL_StatusTypeDef BSP_I2C_WriteBuffer(uint16_t device_addr, uint8_t reg_addr, uint8_t *pData, uint16_t size);
HAL_StatusTypeDef BSP_I2C_ReadBuffer(uint16_t device_addr, uint8_t reg_addr, uint8_t *pData, uint16_t size);

#endif /* __BSP_I2C_H */