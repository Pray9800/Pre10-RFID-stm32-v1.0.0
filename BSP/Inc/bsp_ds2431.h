/*******************************************************
 Copyright (C), Hangzhou Jianjia Robot Co.,Ltd.
 File name:			bsp_ds2431.h
 Author: PENG		Version: V1.0		Date:2025/06/26
 Description:    	DS2431配置头文件
 Function List:
 History:
*******************************************************/

#ifndef INC_BSP_DS2431_H_
#define INC_BSP_DS2431_H_


#include "gpio.h"

extern uint8_t r_DeviceID[8];	//设备ID
extern uint8_t r_EEPROM[144];	//读取到的数据缓存
extern uint8_t w_Page[4][8];
extern uint8_t w_Line[8];
//#define D0_Write_1 		HAL_GPIO_WritePin(D0_GPIO_Port, D0_Pin, GPIO_PIN_SET)
//#define D0_Write_0 		HAL_GPIO_WritePin(D0_GPIO_Port, D0_Pin, GPIO_PIN_RESET)
//#define D0_ReadBit 		HAL_GPIO_ReadPin(D0_GPIO_Port, D0_Pin)

#define D0				0
#define D1				1

#define Write_0(x)   	do{ x? \
                      	  HAL_GPIO_WritePin(D1_GPIO_Port, D1_Pin, GPIO_PIN_RESET): \
                          HAL_GPIO_WritePin(D0_GPIO_Port, D0_Pin, GPIO_PIN_RESET); \
                  	  	}while(0)

#define Write_1(x)   	do{ x? \
                      	  HAL_GPIO_WritePin(D1_GPIO_Port, D1_Pin, GPIO_PIN_SET): \
                          HAL_GPIO_WritePin(D0_GPIO_Port, D0_Pin, GPIO_PIN_SET); \
                  	  	}while(0)



 				


/*DS2431 ROM功能命令*/
#define Rom_Read_Cmd            0x33    //Read ROM
#define Rom_Match_Cmd           0x55    //Match ROM
#define Rom_Skip_Cmd            0xCC    //Skip ROM
#define Rom_Search_Cmd          0xF0    //Search ROM

/*DS2431 存储器功能命令*/
#define Memory_Read_Cmd         0xF0    //Read Memory
#define Scratchpad_Read_Cmd     0xAA    //Read Scratchpad
#define Scratchpad_Write_Cmd    0x0F    //Write Scratchpad
#define Scratchpad_Copy_Cmd     0x55    //Copy Scratchpad


uint8_t DS2431_Init(uint8_t pin);
uint8_t DS2431_ReadID(uint8_t pin, uint8_t *id);
uint8_t DS2431_Read_Data(uint8_t pin, uint8_t tgaddr, uint8_t len, uint8_t *DataBuff);
uint8_t DS2431_Read_Page(uint8_t pin, uint8_t nPage, uint8_t *DataBuff);
uint8_t DS2431_Read_EntireEEPROM(uint8_t pin, uint8_t *DataBuff);
uint8_t DS2431_Write_Line(uint8_t pin, uint8_t nLine, uint8_t *DataBuff);
uint8_t DS2431_Write_Page(uint8_t pin, uint8_t nPage, uint8_t *DataBuff);
uint8_t DS2431_WriteAllPage(uint8_t pin, uint8_t * buffer);




extern uint8_t r_DeviceID[8];//外部调用
extern uint8_t r_EEPROM[144];//外部调用

#endif /* INC_BSP_DS2431_H_ */
