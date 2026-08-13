#include "bsp_usart.h"
#include "string.h"
#include "usart.h"
#include "bsp_tim.h"
#include "bsp_crc.h"


uint8_t rx_temp; // 单字节接收缓存
uint8_t UART2_Rxbuff[50];
 


uint8_t C6_Light_msg[7] = {0xa5,0x5a,0x0c,0x01,0x00,0xb6,0x6b}; //测试




void MCU_485_Receive_IT_Start(uint8_t *pData, uint16_t Size)
{
 
    HAL_UART_Receive_IT(&huart2, pData, Size);
}


void MCU_485_Send(uint8_t *buf,uint8_t len)
{
    // 如果串口还在发送中，等待其完成
    while (huart2.gState == HAL_UART_STATE_BUSY_TX);
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
    for (volatile uint16_t i = 0; i < 500; i++)
    {
        __NOP();
    }

    HAL_UART_Transmit_DMA(&huart2, buf, len);
}


// void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
// {
//     if (huart->Instance == USART2)
//     {
//         HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
//     }
// }


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    // 判断是否是 485 用的串口 (huart2)
    if (huart->Instance == USART2)
    {
        // DMA 和串口发送完全结束，切回接收模式（拉低 DE）
        HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
    }
}







/*******************************************************
 Author: PAN       Version: V1.0       Date:2026/08/12
 Function:          HAL_UART_RxCpltCallback
 Description:       USART2 单字节接收回调
                     仅处理 0x5B 起始帧，缓存到 UART2_Rxbuff[]
                     当接收长度达到 10 字节后做 CRC16_Modbus 校验
                     校验通过后保留帧数据，等待上层处理
 Input:             huart: UART handle
 Output:            无
 Return:            无
 Others:            只保留有效帧存储和 CRC 校验逻辑
*******************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    static uint8_t rx_cnt = 0;

    if (huart->Instance == USART2)
    {
        if (rx_cnt == 0)
    {
        if (rx_temp == 0x5B)
        {
            UART2_Rxbuff[0] = rx_temp;
            rx_cnt = 1;
        }
    }
    else
    {
        UART2_Rxbuff[rx_cnt] = rx_temp;
        rx_cnt++;

        if (rx_cnt >= 10)
        {
            if (UART2_Rxbuff[1] == 0x06)
            {
                uint16_t calc_crc = CRC16_Modbus(UART2_Rxbuff, 8);
                uint16_t recv_crc = (uint16_t)((UART2_Rxbuff[9] << 8) | UART2_Rxbuff[8]);

                if (calc_crc == recv_crc)
                {
                    
                    // 收到合法帧，数据已存入 UART2_Rxbuff
                }
            }

            rx_cnt = 0;
        }
        }
    }

    MCU_485_Receive_IT_Start(&rx_temp, 1);
}


