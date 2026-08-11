/*******************************************************
 Copyright (C), Hangzhou Jianjia Robot Co.,Ltd.
 File name:			bsp_ds2431.c
 Author: PENG		Version: V1.0		Date:2025/06/26
 Description:    	DS2431配置文件
 Function List:
 History:
*******************************************************/

//#include "task.h"
#include "bsp_ds2431.h"
#include "bsp_tim.h"
//#include "bsp_tea.h"
#include "gpio.h"
//#include "cmsis_os.h"

uint8_t r_DeviceID[8] = {0};	//设备ID
uint8_t r_EEPROM[144] = {0};	//读取到的数据缓存

uint8_t w_Line[8] = 	{		//待发送的数据缓存
        0x01,0x02,0x03,0x00,0x00,0x00,0x00,0x00,
};

uint8_t w_Page[4][8] =	{		//待发送的数据缓存

        {0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00},
        {0x02,0x03,0x00,0x00,0x00,0x00,0x00,0x00},
        {0x03,0x04,0x00,0x00,0x00,0x00,0x00,0x00},
        {0x04,0x05,0x00,0x00,0x00,0x00,0x00,0x00},
};


/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			ReadBit
 Description:		读取总线数据位
 Calls:
 Called By:
 Input:				无
 Output:			无
 Return:			无
 Others:			无
*******************************************************/
uint8_t ReadBit(uint8_t pin)
{
	if(pin == D0)
	{
		return HAL_GPIO_ReadPin(D0_GPIO_Port, D0_Pin);
	}
	else
	{
		return HAL_GPIO_ReadPin(D1_GPIO_Port, D1_Pin);
	}
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			Set_OUTPUT
 Description:		设置总线方向为输出
 Calls:
 Called By:
 Input:				无
 Output:			无
 Return:			无
 Others:			无
*******************************************************/
static void Set_OUTPUT(uint8_t pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if(pin == D0)
    {
        // 设置引脚为开漏输出模式
		GPIO_InitStruct.Pin = D0_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(D0_GPIO_Port, &GPIO_InitStruct);
		// 默认设置为高电平（释放总线）
		HAL_GPIO_WritePin(D0_GPIO_Port, D0_Pin, GPIO_PIN_SET);
    }
    else if(pin == D1)
    {
        // 设置引脚为开漏输出模式
		GPIO_InitStruct.Pin = D1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(D0_GPIO_Port, &GPIO_InitStruct);
		// 默认设置为高电平（释放总线）
		HAL_GPIO_WritePin(D1_GPIO_Port, D1_Pin, GPIO_PIN_SET);
    }
}


/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			Set_INPUT
 Description:		设置总线方向为输入
 Calls:
 Called By:
 Input:				无
 Output:			无
 Return:			无
 Others:			无
*******************************************************/
static void Set_INPUT(uint8_t pin)
{
	if(pin == D0)
	{
		// 设置引脚为输入模式
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = D0_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(D0_GPIO_Port, &GPIO_InitStruct);
	}
	else if(pin == D1)
	{
		// 设置引脚为输入模式
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = D1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(D1_GPIO_Port, &GPIO_InitStruct);
	}
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DeviceReset
 Description:		器件复位
 Calls:
 Called By:
 Input:				无
 Output:			无
 Return:			0: 有应答
 	 	 	 	 	1: 无应答
 Others:			无
*******************************************************/
static uint8_t DeviceReset(uint8_t pin)
{
//	vTaskSuspendAll();
	Set_OUTPUT(pin);
	Write_1(pin);
	Write_0(pin);
	delay_us(500);		//复位低脉冲保持至少480us

	Write_1(pin);
	delay_us(100);		//释放总线后100us读应答

	Set_INPUT(pin);
	if(0 == ReadBit(pin))
	{
		delay_us(500);	//tRSTH在标准速度下最小应为480μs
//		xTaskResumeAll();
		return (0);
	}
	else
	{
//		xTaskResumeAll();
		return (1);
	}
}


/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DeviceWriteBit_1
 Description:		写数据位1
 Calls:
 Called By:
 Input:				无
 Output:			无
 Return:			无
 Others:			无
*******************************************************/
static void DeviceWriteBit_1(uint8_t pin)
{
//	vTaskSuspendAll();
	Set_OUTPUT(pin);
	Write_1(pin);
	Write_0(pin);		//拉低总线
	delay_us(5);		//拉低总线保持5us
	Write_1(pin);		//释放总线
	delay_us(65);		//释放总线后延时65us
//	xTaskResumeAll();
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DeviceWriteBit_0
 Description:		写数据位0
 Calls:
 Called By:
 Input:				无
 Output:			无
 Return:			无
 Others:			无
*******************************************************/
static void DeviceWriteBit_0(uint8_t pin)
{
//	vTaskSuspendAll();
	Set_OUTPUT(pin);
	Write_1(pin);
	Write_0(pin);		//拉低总线
	delay_us(65);		//拉低总线保持65us
	Write_1(pin);		//释放总线
	delay_us(5);		//释放总线延时5us
//	xTaskResumeAll();
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DeviceReadBit
 Description:		读数据位
 Calls:
 Called By:
 Input:				无
 Output:			无
 Return:			无
 Others:			无
*******************************************************/
static uint8_t DeviceReadBit(uint8_t pin)
{
	uint8_t value;

//	vTaskSuspendAll();
	Set_OUTPUT(pin);
	Write_1(pin);
	Write_0(pin);		//拉低总线
	delay_us(5);		//拉低总线保持5us
	Write_1(pin);		//释放总线
	delay_us(5);

	Set_INPUT(pin);
	value = ReadBit(pin);
	delay_us(55);		//读取总线状态延时55us
//	xTaskResumeAll();

	return value;
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DeviceWrite_Byte
 Description:		写字节
 Calls:
 Called By:
 Input:				无
 Output:			无
 Return:			无
 Others:			无
*******************************************************/
static void DeviceWrite_Byte(uint8_t pin, uint8_t value)
{
	uint8_t i;

	for(i=0; i<8; i++)
	{
		if(value & 0x01)
		{
			DeviceWriteBit_1(pin);
		}
		else
		{
			DeviceWriteBit_0(pin);
		}
		value >>= 1;
	}
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DeviceRead_Byte
 Description:		读字节
 Calls:
 Called By:
 Input:				无
 Output:			无
 Return:			无
 Others:			无
*******************************************************/
static uint8_t DeviceRead_Byte(uint8_t pin)
{
	uint8_t i;
	uint8_t value = 0;

	for(i=0; i<8; i++)
	{
		value >>= 1;
		if(DeviceReadBit(pin))
		{
			value |= 0x80;
		}
	}

	return value;
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			CRC_Check
 Description:		8位CRC8/MAXIM校验 多项式:X8+X5+X4+1
 Calls:
 Called By:
 Input:				无
 Output:			无
 Return:			无
 Others:			无
*******************************************************/
uint8_t CRC_Check(uint8_t *message, uint8_t len)
{
	uint8_t i;
	uint8_t crc = 0;

	while(len--)
	{
		crc ^= *message++;

		for(i=0; i<8; i++)
		{
			if(crc & 0x01)
			{
				crc = (crc >> 1) ^ 0x8c;
			}
			else
			{
				crc >>= 1;
			}
		}
	}

	return crc;
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DS2431_Init
 Description:		DS2431初始化
 Calls:
 Called By:
 Input:				无
 Output:			无
 Return:			无
 Others:			无
*******************************************************/
uint8_t DS2431_Init(uint8_t pin)
{
	Set_OUTPUT(pin);

	return DeviceReset(pin);
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DS2431_ReadID
 Description:		读8位家族码 48位序列号 8位CRC码  共8个字节
 	 	 	 	 	DS2431 有两种不同类型的CRC码，如下：
						一种为8位，存储在64位ROM的最高字节中。主机能根
					据64位ROM码的前56位计算出该CRC码，并与存储在DS2431 中的值比较，判断ROM数据是
					否接收无误。计算该CRC校验码的等效多项式为：X8 + X5 + X4 + 1。接收到的8位CRC为
					原码（不取反）形式。该值在工厂计算并被光刻写入ROM中。
						另一种CRC码为16位，采用标准的CRC16多项式函数：X16 + X15  + X2  + 1 产生。
					该CRC校验码用来对读写暂存器时传输的数据进行快速校验。与8位CRC校验码不同，16位
					CRC校验码总是以反码的形式传输。
					Read ROM 此命令只用于读取DS2431的8位家族码，唯一的48ID号和8位CRC校验码。
 Calls:
 Called By:
 Input:				无
 Output:			无
 Return:			无
 Others:			无
*******************************************************/
uint8_t DS2431_ReadID(uint8_t pin, uint8_t *id)
{
	uint8_t i, crcTemp;

	if(DeviceReset(pin))	//复位总线
	{
		return (1);
	}

	/*发送Read ROM*/
	DeviceWrite_Byte(pin, Rom_Read_Cmd);
	delay_us(100);

	/*读取DS2431的8位家族码 + 48位ID + 8位CRC校验码*/
	for(i=0; i<8; i++)
	{
		id[i] = DeviceRead_Byte(pin);
	}

	if(DeviceReset(pin))	//复位总线
	{
		return (1);
	}

	/*CRC8校验，此处采用的是第一种8位码的CRC校验方式*/
	crcTemp = CRC_Check(&id[0], 7);
	if(crcTemp != id[7])
	{
		return (2);
	}

	return (0);
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DS2431_Read_Data
 Description:		读指定长度的数据
 Calls:
 Called By:
 Input:				DataBuff: 数据缓存指针 数据缓存至少要有128字节
 Output:			无
 Return:			0: 成功
 	 	 	 	 	1: 总线不可用
 Others:			无
*******************************************************/
uint8_t DS2431_Read_Data(uint8_t pin, uint8_t tgaddr, uint8_t len, uint8_t *DataBuff)
{
	uint8_t i;

	if(DeviceReset(pin))	//复位总线
	{
		return (1);
	}

    DeviceWrite_Byte(pin, Rom_Skip_Cmd);		//写命令Skip ROM
    DeviceWrite_Byte(pin, Memory_Read_Cmd);		//写命令Read Memory
    DeviceWrite_Byte(pin, tgaddr);				//写地址低字节TA1
    DeviceWrite_Byte(pin, 0);					//写地址高字节TA2(always zero for DS2431)
	delay_us(100);

    for(i=0; i<len; i++)
    {
    	DataBuff[i] = DeviceRead_Byte(pin);
    }

	if(DeviceReset(pin))	//复位总线
	{
		return (1);
	}

    return (0);
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DS2431_Read_Page
 Description:		读DS2431的数据存储器某一页 共32个字节
 	 	 	 	 	数据存储器页和寄存器行对读操作没有限制
 Calls:
 Called By:
 Input:				nPage: 要读哪一页，取值范围0~3
 	 	 	 	 	DataBuff:数据缓存指针 数据缓存至少要有8字节
 Output:			无
 Return:			0: 成功
 	 	 	 	 	1: 总线不可用
 Others:			无
*******************************************************/
uint8_t DS2431_Read_Page(uint8_t pin, uint8_t nPage, uint8_t *DataBuff)
{
	uint8_t i;

	if(nPage > 3)			//页码超限
	{
		return (4);
	}

	if(DeviceReset(pin))	//复位总线
	{
		return (1);
	}

	DeviceWrite_Byte(pin, Rom_Skip_Cmd);		//写命令Skip ROM
	DeviceWrite_Byte(pin, Memory_Read_Cmd);		//写命令Read Memory
	DeviceWrite_Byte(pin, (nPage<<5) & 0xFF);	//写地址低字节TA1
	DeviceWrite_Byte(pin, 0);					//写地址高字节TA2(always zero for DS2431)
	delay_us(100);

	for (i=0; i<32; i++)
	{
		*(DataBuff++) = DeviceRead_Byte(pin);
	}

	if(DeviceReset(pin))	//复位总线
	{
		return (1);
	}

	return (0);
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DS2431_Init
 Description:		读2431的存储器EEPROM阵列 共144个字节
 Calls:
 Called By:
 Input:				DataBuff: 数据缓存指针（数据缓存至少要有144字节）
 Output:			无
 Return:			0: 成功
 	 	 	 	 	1: 总线不可用
 Others:			无
*******************************************************/
uint8_t DS2431_Read_EntireEEPROM(uint8_t pin, uint8_t *DataBuff)
{
	uint8_t i;

	if(DeviceReset(pin))			//复位总线
	{
		return (1);
	}

	DeviceWrite_Byte(pin, Rom_Skip_Cmd);	//写命令Skip ROM
	DeviceWrite_Byte(pin, Memory_Read_Cmd);	//写命令Read Memory
	DeviceWrite_Byte(pin, 0);				//写地址低字节TA1
	DeviceWrite_Byte(pin, 0);				//写地址高字节TA2(always zero for DS2431)
	delay_us(100);

	for(i=0; i<144; i++)
	{
		*(DataBuff++) = DeviceRead_Byte(pin);
	}

	if(DeviceReset(pin))			//复位总线
	{
		return (1);
	}

	return (0);
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DS2431_Write_Line
 Description:		写D2431的数据存储器的某一行 一行总共8个字节
 Calls:
 Called By:
 Input:				nLine: 要写哪一行，取值范围(0-17)，其中16行为特殊功能寄存器，17行为保留
 	 	 	 	 	DataBuff: 数据缓存指针 数据缓存至少要有8字节
 Output:			无
 Return:			0: 成功
 	 	 	 	 	1: 总线不可用
 	 	 	 	 	2: 写暂存器失败
 	 	 	 	 	3: 写主存储器失败
 Others:			无
*******************************************************/
uint8_t DS2431_Write_Line(uint8_t pin, uint8_t nLine, uint8_t *DataBuff)
{
	uint8_t i;
	uint8_t TA1, TA2, E_S;
	uint8_t sbuf[8];

	if(nLine > 16)			//行码超限
	{
		return (4);
	}

	if(DeviceReset(pin))	//复位总线
	{
		return (1);
	}
	DeviceWrite_Byte(pin, Rom_Skip_Cmd);
	DeviceWrite_Byte(pin, Scratchpad_Write_Cmd);
	DeviceWrite_Byte(pin, (nLine<<3) & 0xFF);	//写地址低字节TA1
	DeviceWrite_Byte(pin, 0);					//写地址高字节TA2(always zero for DS2431)
	delay_us(100);
	for(i=0; i<8; i++)
	{
		DeviceWrite_Byte(pin, DataBuff[i]);
	}
	/* crc字节读取，但未进行校验，TODO:需要实现CRC16多项式函数：X16+X15+X2+1 */
	DeviceRead_Byte(pin);
	DeviceRead_Byte(pin);
	delay_us(200);


	if(DeviceReset(pin))	//复位总线
	{
		return (1);
	}
	DeviceWrite_Byte(pin, Rom_Skip_Cmd);
	DeviceWrite_Byte(pin, Scratchpad_Read_Cmd);
	delay_us(100);
	/*获取授权码*/
	TA1 = DeviceRead_Byte(pin);
	TA2 = DeviceRead_Byte(pin);
	E_S = DeviceRead_Byte(pin);
	/*校验授权码*/
	if(TA1 != ((nLine<<3) & 0xFF))
	{
		return (2);
	}
	else if(TA2 != 0)				//TA2(always 0 for DS2431)
	{
		return (2);
	}
	else if(E_S != 7)				//E_S(always 7 for DS2431)
	{
		return (2);
	}
	delay_us(10);
	for(i=0; i<8; i++)				//读8个数据
	{
		sbuf[i] = DeviceRead_Byte(pin);
		if(sbuf[i] != DataBuff[i])
		{
			return (2);
		}
	}
	/* crc字节读取，但未进行校验，TODO:需要实现CRC16多项式函数：X16+X15+X2+1 */
	DeviceRead_Byte(pin);
	DeviceRead_Byte(pin);


	if(DeviceReset(pin))			//复位总线
	{
		return (1);
	}
	DeviceWrite_Byte(pin, Rom_Skip_Cmd);
	DeviceWrite_Byte(pin, Scratchpad_Copy_Cmd);
	delay_us(100);
	//发送授权码
	DeviceWrite_Byte(pin, TA1);
	DeviceWrite_Byte(pin, TA2);
	DeviceWrite_Byte(pin, E_S);
	Sys_Delay(15);					//等待tPROGmax 完成复制操作
	if(DeviceRead_Byte(pin) != 0xAA)//读复制状态，AAh = 成功
	{
		return (3);
	}

	if(DeviceReset(pin))			//复位总线
	{
		return (1);
	}

	return (0);
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DS2431_Write_Page
 Description:		写DS2431的指定页 共32个字节
 Calls:
 Called By:
 Input:				nPage: 要写哪一页，取值范围0~2, 最后一页不可整页写
 	 	 	 	 	DataBuff: 数据缓存指针 数据缓存至少要有32字节
 Output:			无
 Return:			0: 成功
 	 	 	 	 	1: 失败
 Others:			无
*******************************************************/
uint8_t DS2431_Write_Page(uint8_t pin, uint8_t nPage, uint8_t *DataBuff)
{
	uint8_t i;

	if(nPage > 2)		//页码超限
	{
		return (1);
	}

	for(i=0; i<4; i++)	//1页包含4行
	{
		if(DS2431_Write_Line(pin, nPage*4+i, DataBuff+i*8) != 0)
		{
			return (1);
		}
	}
	return (0);
}

/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			DS2431_Write_nLine
 Description:		写2431的全部数据存储器
 Calls:
 Called By:
 Input:				DataBuff: 数据缓存指针 数据缓存至少要有128字节
 Output:			无
 Return:			0: 成功
 	 	 	 	 	1: 总线不可用
 Others:			无
*******************************************************/
uint8_t DS2431_WriteAllPage(uint8_t pin, uint8_t *buffer)
{
	uint8_t i;

	for(i=0; i<16; i++)	//2431的全部数据存储器占前16行
	{
		if(DS2431_Write_Line(pin, i, buffer+i*8) != 0)
		{
			return (1);
		}
	}
	return (0);
}


