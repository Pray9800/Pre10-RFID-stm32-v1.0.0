
 
#include "main.h"          
#include "stm32g4xx.h"




// DWT 初始化函数
void  DWT_Init(void)
{
    // 使能 TRC (Trace) 模块 追踪
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // 清零 DWT 周期计数器
    DWT->CYCCNT = 0;
    // 使能 DWT 周期计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
  * @brief  微秒级延时函数 (主频 100MHz)
  * @param  us: 需要延时的微秒数
  */
void delay_us(uint32_t us)
{
    uint32_t start_cycles = DWT->CYCCNT;
    uint64_t delay_cycles = ((uint64_t)SystemCoreClock * us) / 1000000U;

    while ((uint32_t)(DWT->CYCCNT - start_cycles) < delay_cycles)
    {
        // 等待 CPU 时钟周期达到要求
    }
}

/*******************************************************
 Author: PAN        Version: V1.0       Date:2026/05/11
 Function:          Key_Scan_Once
 Description:       毫秒延时（ms）
 Input:             ms           
 Output:            按键代表的数值
 Return:            无
 Others:            无
*******************************************************/
void Sys_Delay(uint16_t time_ms)

{
   HAL_Delay(time_ms); 
}
