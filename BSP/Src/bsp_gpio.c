
#include "bsp_gpio.h"
#include "bsp_rc522.h"


/*******************************************************
 Author: PAN       Version: V1.0       Date:2026/05/11
 Function:          Key_Scan_Once
 Description:       闪烁10hz 检验程序是否正常运行
 Input:             
 Output:            按键代表的数值
 Return:            无
 Others:            无
*******************************************************/
void blink()
{
    HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
    //Sys_Delay(50);

}



// /**
//   * @brief  EXTI 外部中断回调函数 (HAL 库重弱定义函数)
//   * @param  GPIO_Pin: 触发中断的 GPIO 引脚编号 (例如 GPIO_PIN_0)
//   * @retval None
//   */
// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
// {
//     // 1. 确认触发中断的是不是 PB0 (IQR_Pin)
//     if (GPIO_Pin == IQR_Pin) // 或使用 RC522_IRQ_PIN / GPIO_PIN_0
//     {
//         /* --- USER CODE BEGIN EXTI0 --- */
        
//         //  读取 RC522 的 ComIrqReg (0x04) 寄存器，查询具体发生了什么中断
//         uint8_t irq_status = BSP_RC522_ReadReg(0x04);

//         // 判断是不是接收到卡片响应中断 (RxIRq, 即 Bit 5)
//         if (irq_status & 0x20)
//         {
//             // 刷卡成功！可以设置全局标志位，或者通知主任务去读卡号
//             // 例如: g_card_detected_flag = 1;
//         }

//         // 清除 RC522 芯片内部的中断标志位 (写入 0x7F)
      
//         BSP_RC522_WriteReg(0x04, 0x7F);

//         /* --- USER CODE END EXTI0 --- */
//     }
// }





/**
  * @brief  STM32 GPIO 外部中断通用回调函数
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* 判断是否是 RC522 的 IRQ 引脚触发了中断 */
    if (GPIO_Pin == RC522_IRQ_PIN)
    {
        /* 1. 读取 RC522 的中断状态寄存器 (CommIrqReg) */
        uint8_t irq_status = BSP_RC522_ReadReg(MFRC_ComIrqReg);

        /* 2. 清除中断标志，防止反复进入中断 */
        BSP_RC522_WriteReg(MFRC_ComIrqReg, MFRC_CommIrqClear);

        /* 3. 判断是否是寻卡/接收数据完成中断 (RxIRq) */
        if (irq_status & MFRC_RxIRq)
        {
            /* 触发了卡片读写中断，可在此时通知 task() 去执行防冲撞与读卡号逻辑！ */
        }
    }
}
