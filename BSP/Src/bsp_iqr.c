#include "bsp_iqr.h"
#include "bsp_rc522.h"


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
