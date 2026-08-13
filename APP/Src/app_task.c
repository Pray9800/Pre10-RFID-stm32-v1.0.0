#include "app_task.h"
#include "bsp_rc522.h"
#include "bsp_tim.h"
#include "bsp_ds2431.h"
#include "bsp_usart.h"

uint8_t rc522_ready = 0;
uint8_t version = 0;

void task_init()
{
    DWT_Init();
    rc522_ready = BSP_RC522_IsDeviceReady();
    version = BSP_RC522_ReadReg(0x37);
    //onewire模式下单线测试
    DS2431_Init(D0);
    DS2431_ReadID(D0, r_DeviceID);
    //测试写入数据 
    uint8_t test_data[8]={0xAA,0XAB,0XCC,0XDD,0XEE,0XFF,0X11,0X22};
    DS2431_Write_Line(D0, 1, test_data);  //page 0的第二行
    //DS2431_Write_Page(D0, 1, test_data);

}

void task()
{
        task_init(); //初始化
        //
        while(1)
        {
            if(DS2431_ReadID(D0, r_DeviceID) == 0)  // 返回0=成功
            {
                // 设备连接，读数据
                DS2431_Read_Data(D0,1*8,8,r_EEPROM);  //读取page1的第一行数据
                // DS2431_Read_Page(D0, 1, r_EEPROM);
                HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
                HAL_Delay(100);
                HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
                HAL_Delay(100);      
                MCU_485_Send(r_EEPROM,8);           
            } 
      
        }
      

}

