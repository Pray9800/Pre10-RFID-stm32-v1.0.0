/*******************************************************
 Copyright (C), HangZhou Jianjia Co.,Ltd.
 File name:       bsp_i2c.c
 Author: PAN      Version: V1.0      Date: 2026/05/15
 Description:     基于 I2C2 的 RC522 读卡器底层 BSP 实现
*******************************************************/

#include "bsp_i2c.h"

/**
  * @brief  检测 I2C 设备是否响应
  * @param  device_addr: 设备 I2C 地址（HAL 库格式）
  * @retval 1: 响应正常 (ACK), 0: 无响应
  */
uint8_t BSP_I2C_IsDeviceReady(uint16_t device_addr)
{
  if (HAL_I2C_IsDeviceReady(&hi2c2, device_addr, 3, BSP_I2C_TIMEOUT) == HAL_OK)
    {
        // printf("RC522 I2C device is ready.\r\n");
        return 1;
    }
    return 0;
}

/**
  * @brief  向 I2C 设备单个寄存器写入数据
  * @param  device_addr: 设备 I2C 地址（HAL 库格式）
  * @param  reg_addr: 目标寄存器地址
  * @param  value: 写入的数据字节
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_I2C_WriteReg(uint16_t device_addr, uint8_t reg_addr, uint8_t value)
{
  return HAL_I2C_Mem_Write(&hi2c2, device_addr, (uint16_t)reg_addr, 
                            I2C_MEMADD_SIZE_8BIT, &value, 1, BSP_I2C_TIMEOUT);
}

/**
  * @brief  从 I2C 设备单个寄存器读取数据
  * @param  device_addr: 设备 I2C 地址（HAL 库格式）
  * @param  reg_addr: 目标寄存器地址
  * @retval 读取到的 1 字节数据
  */
uint8_t BSP_I2C_ReadReg(uint16_t device_addr, uint8_t reg_addr)
{
    uint8_t value = 0;
  HAL_I2C_Mem_Read(&hi2c2, device_addr, (uint16_t)reg_addr, 
                     I2C_MEMADD_SIZE_8BIT, &value, 1, BSP_I2C_TIMEOUT);
    return value;
}

/**
  * @brief  向 I2C 设备指定寄存器连续写入多字节数据
  * @param  device_addr: 设备 I2C 地址（HAL 库格式）
  * @param  reg_addr: 目标起始寄存器地址
  * @param  pData: 待发送数据缓冲区指针
  * @param  size: 发送数据字节长度
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_I2C_WriteBuffer(uint16_t device_addr, uint8_t reg_addr, uint8_t *pData, uint16_t size)
{
  return HAL_I2C_Mem_Write(&hi2c2, device_addr, (uint16_t)reg_addr, 
                            I2C_MEMADD_SIZE_8BIT, pData, size, BSP_I2C_TIMEOUT);
}

/**
  * @brief  从 I2C 设备指定寄存器连续读取多字节数据
  * @param  device_addr: 设备 I2C 地址（HAL 库格式）
  * @param  reg_addr: 目标起始寄存器地址
  * @param  pData: 接收数据缓冲区指针
  * @param  size: 读取数据字节长度
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_I2C_ReadBuffer(uint16_t device_addr, uint8_t reg_addr, uint8_t *pData, uint16_t size)
{
  return HAL_I2C_Mem_Read(&hi2c2, device_addr, (uint16_t)reg_addr, 
                     I2C_MEMADD_SIZE_8BIT, pData, size, BSP_I2C_TIMEOUT);
}

