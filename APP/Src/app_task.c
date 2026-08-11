#include "app_task.h"
#include "bsp_rc522.h"
#include "bsp_tim.h"
uint8_t rc522_ready = 0;
uint8_t version = 0;

void task_init()
{
      DWT_Init();
    rc522_ready = BSP_RC522_IsDeviceReady();
    version = BSP_RC522_ReadReg(0x37);

}

void task()
{
        task_init(); //初始化
        while (1)
        {
            HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
            HAL_Delay(100);
        }
        

}

