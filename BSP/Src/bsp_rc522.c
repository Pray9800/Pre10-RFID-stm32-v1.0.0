/*******************************************************
 * RC522 专用 BSP 实现
 *******************************************************/

#include "bsp_rc522.h"
#include "string.h"
uint8_t BSP_RC522_IsDeviceReady(void)
{
    return BSP_I2C_IsDeviceReady(RC522_I2C_ADDR);
}

HAL_StatusTypeDef BSP_RC522_WriteReg(uint8_t reg_addr, uint8_t value)
{
    return BSP_I2C_WriteReg(RC522_I2C_ADDR, reg_addr, value);
}

uint8_t BSP_RC522_ReadReg(uint8_t reg_addr)
{
    return BSP_I2C_ReadReg(RC522_I2C_ADDR, reg_addr);
}

HAL_StatusTypeDef BSP_RC522_WriteBuffer(uint8_t reg_addr, uint8_t *pData, uint16_t size)
{
    return BSP_I2C_WriteBuffer(RC522_I2C_ADDR, reg_addr, pData, size);
}

HAL_StatusTypeDef BSP_RC522_ReadBuffer(uint8_t reg_addr, uint8_t *pData, uint16_t size)
{
    return BSP_I2C_ReadBuffer(RC522_I2C_ADDR, reg_addr, pData, size);
}




uint8_t BSP_RC522_ReadIRQPin(void)
{
    return (uint8_t)HAL_GPIO_ReadPin(RC522_IRQ_PORT, RC522_IRQ_PIN);
}




/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/08/14
 Function:			MFRC_ClrBitMask
 Description:		清除寄存器的位
 Calls:
 Called By:
 Input:				addr: 待清除的寄存器地址
 	 	 	 	 	mask: 待清除寄存器的位 可同时清除多个bit
 Output:
 Return:
 Others:
*******************************************************/
void RC522_ClrBitMask(uint8_t addr, uint8_t mask)
{
    uint8_t temp;
    temp = BSP_RC522_ReadReg(addr);				//先读回寄存器的值
    BSP_RC522_WriteReg(addr, temp & (~mask));	//处理过的数据再写入寄存器
}




void RC522_SetBitMask(uint8_t addr, uint8_t mask)
{
    uint8_t temp;
    temp = BSP_RC522_ReadReg(addr);			//先读回寄存器的值
    BSP_RC522_WriteReg(addr, temp | mask);	//处理过的数据再写入寄存器
}



/*******************************************************
 Author: PAN        Version: V1.0       Date:2026/08/14
 Function:          RC522_CalulateCRC
 Description:       用RC522硬件计算CRC结果
                    用于ISO14443A RFID通信协议的数据校验

 ISO14443A CRC-A 规范：
 ┌────────────────────────────────────────────────────┐
 │ 参数          │ 值        │ 说明                   │
 ├────────────────────────────────────────────────────┤
 │ 多项式        │ 0x1021    │ x^16+x^12+x^5+1       │
 │ 初始值        │ 0x6363    │ ISO14443A标准规定      │
 │ 输入反转      │ 否        │                        │
 │ 输出反转      │ 否        │                        │
 │ 最终异或      │ 0x0000    │ 不异或                 │
 └────────────────────────────────────────────────────┘

 RC522 CRC硬件计算流程：
 ┌────────────────────────────────────────────────────┐
 │                                                    │
 │  1. 准备工作                                       │
 │     ├── 使能CRC中断 (DivIrqReg BIT2)              │
 │     ├── 取消当前命令                               │
 │     └── 清空FIFO缓冲区                            │
 │                                                    │
 │  2. 写入数据                                       │
 │     └── 将待计算数据逐字节写入FIFO                 │
 │                                                    │
 │  3. 启动CRC计算                                    │
 │     └── 写CommandReg = CALCCRC (0x03)             │
 │                                                    │
 │  4. 等待完成                                       │
 │     ├── 循环读取DivIrqReg                         │
 │     └── 等待CRCIRq标志 (BIT2 = 1)                 │
 │                                                    │
 │  5. 读取结果                                       │
 │     ├── CRCResultRegL: CRC低字节                  │
 │     └── CRCResultRegM: CRC高字节                  │
 │                                                    │
 └────────────────────────────────────────────────────┘

 Calls:             BSP_RC522_ReadReg、BSP_RC522_WriteReg、RC522_SetBitMask、RC522_ClrBitMask
 Called By:         PCD_Select、PCD_WriteBlock、PCD_ReadBlock、PCD_Halt
 Input:             pInData: 待进行CRC计算的数据
                    len: 待进行CRC计算的数据长度
 Output:            pOutData: CRC计算结果（2字节）
 Return:            无
 Others:            CRC结果低字节在前，高字节在后

 注意：此函数使用RC522硬件CRC，与软件CRC16_Modbus不同！
       - CRC16_Modbus: 用于RS485/Modbus通信
       - RC522_CalulateCRC: 用于RFID ISO14443A通信
*******************************************************/
void RC522_CalulateCRC(uint8_t *pInData, uint8_t len, uint8_t *pOutData)
{
    uint8_t temp;
    uint32_t i;

    /* Step 1: 准备工作 */
    RC522_ClrBitMask(MFRC_DivIrqReg, 0x04);          // 使能CRC中断 (Clear CRCIRq)
    BSP_RC522_WriteReg(MFRC_CommandReg, MFRC_IDLE);  // 取消当前命令
    RC522_SetBitMask(MFRC_FIFOLevelReg, 0x80);       // 清空FIFO及其标志位

    /* Step 2: 将待CRC计算的数据写入FIFO */
    for(i=0; i<len; i++)
    {
        BSP_RC522_WriteReg(MFRC_FIFODataReg, *(pInData + i));
    }

    /* Step 3: 启动CRC计算 */
    BSP_RC522_WriteReg(MFRC_CommandReg, MFRC_CALCCRC);  // 执行CRC计算

    /* Step 4: 等待CRC计算完成 */
    i = 1000;
    do{
        temp = BSP_RC522_ReadReg(MFRC_DivIrqReg);  // 读取中断标志
        i--;
    }while((i != 0) && !(temp & 0x04));  // 等待CRCIRq标志 (BIT2)

    /* Step 5: 读取CRC结果 */
    pOutData[0] = BSP_RC522_ReadReg(MFRC_CRCResultRegL);  // CRC低字节
    pOutData[1] = BSP_RC522_ReadReg(MFRC_CRCResultRegM);  // CRC高字节
}



/* 开启天线函数依赖的底层改写示例 */
void PCD_AntennaOn(void)
{
    uint8_t temp;
    
    // 读取当前 TxControlReg 的值
    temp = BSP_RC522_ReadReg(MFRC_TxControlReg);
    
    // 如果 TX1 和 TX2 输出未使能（BIT0 和 BIT1），则置 1 开启
    if (!(temp & 0x03))
    {
        BSP_RC522_WriteReg(MFRC_TxControlReg, temp | 0x03);
    }
}

/* 关闭天线函数 */
void PCD_AntennaOff(void)
{
   uint8_t temp;
   temp= BSP_RC522_ReadReg(MFRC_TxControlReg);
   if(temp & 0x03) //如果 TX1 或 TX2 输出使能（BIT0 或 BIT1），则清零关闭
    // 清除 BIT0 和 BIT1，关闭射频输出
    {
        BSP_RC522_WriteReg(MFRC_TxControlReg, temp & ~0x03);
    }
}












/*******************************************************
 Author: PAN        Version: V1.0       Date:2026/08/14
 Function:          RC522_CmdFrame
 Description:       RC522和ISO14443A卡通讯的命令帧函数
 Calls:
 Called By:
 Input:             cmd: RC522命令字
                    pInData: RC522发送给MF1卡的数据的缓冲区首地址
                    InLenByte: 发送数据的字节长度
                    pOutData: 用于接收MF1卡片返回数据的缓冲区首地址
                    pOutLenBit: MF1卡返回数据的位长度
 Output:            pOutData: 用于接收MF1卡片返回数据的缓冲区首地址
                    pOutLenBit: 用于MF1卡返回数据位长度的首地址
 Return:            status: 错误代码(MFRC_OK、MFRC_NOTAGERR、MFRC_ERR)
 Others:
*******************************************************/
char RC522_CmdFrame(uint8_t cmd, uint8_t *pInData, uint8_t InLenByte, uint8_t *pOutData, uint16_t *pOutLenBit)
{
    uint8_t lastBits;
    uint8_t n;
    uint32_t i;
    char status = MFRC_ERR;
    uint8_t irqEn   = 0x00;
    uint8_t waitFor = 0x00;

    /* 根据命令设置标志位 */
    switch(cmd)
    {
        case MFRC_AUTHENT:                  // Mifare认证
            irqEn =   0x12;
            waitFor = 0x10;                 // idleIRq中断标志
            break;
        case MFRC_TRANSCEIVE:               // 发送并接收数据
            irqEn =   0x77;
            waitFor = 0x30;                 // RxIRq和idleIRq中断标志
            break;
        default:
            break;
    }

    /* 发送命令帧前准备 */
    BSP_RC522_WriteReg(MFRC_ComIEnReg, irqEn|0x80);     // 开中断
    RC522_ClrBitMask(MFRC_ComIrqReg, 0x80);              // 清除中断标志位SET1
    BSP_RC522_WriteReg(MFRC_CommandReg, MFRC_IDLE);      // 取消当前命令的执行
    RC522_SetBitMask(MFRC_FIFOLevelReg, 0x80);           // 清除FIFO缓冲区及其标志位

    /* 发送命令帧 */
    for(i = 0; i < InLenByte; i++)                       // 写入命令参数
    {
        BSP_RC522_WriteReg(MFRC_FIFODataReg, pInData[i]);
    }
    BSP_RC522_WriteReg(MFRC_CommandReg, cmd);            // 执行命令
    if(cmd == MFRC_TRANSCEIVE)
    {
        RC522_SetBitMask(MFRC_BitFramingReg, 0x80);      // 启动发送
    }

    i = 2000;   // 根据时钟频率调整,操作M1卡最大等待时间25ms
    do{
        n = BSP_RC522_ReadReg(MFRC_ComIrqReg);
        i--;
    }while((i != 0) && !(n & 0x01) && !(n & waitFor));   // 等待命令完成

    RC522_ClrBitMask(MFRC_BitFramingReg, 0x80);          // 停止发送

    /* 处理接收的数据 */
    if(i != 0)
    {
        if(!(BSP_RC522_ReadReg(MFRC_ErrorReg) & 0x1B))
        {
            status = MFRC_OK;
            if(n & irqEn & 0x01)
            {
                status = MFRC_NOTAGERR;
            }
            if(cmd == MFRC_TRANSCEIVE)
            {
                n = BSP_RC522_ReadReg(MFRC_FIFOLevelReg);
                lastBits = BSP_RC522_ReadReg(MFRC_ControlReg) & 0x07;
                if (lastBits)
                {
                    *pOutLenBit = (n-1)*8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n*8;
                }
                if(n == 0)
                {
                    n = 1;
                }
                if(n > MFRC_MAXRLEN)
                {
                    n = MFRC_MAXRLEN;
                }
                for(i=0; i<n; i++)
                {
                    pOutData[i] = BSP_RC522_ReadReg(MFRC_FIFODataReg);
                }
            }
        }
        else
        {
            status = MFRC_ERR;
        }
    }

    RC522_SetBitMask(MFRC_ControlReg, 0x80);             // 停止定时器运行
    BSP_RC522_WriteReg(MFRC_CommandReg, MFRC_IDLE);       // 取消当前命令的执行

    return status;
}








void RFID_RC522_Init(void)
{
    //软件传输复位功能  命令寄存器
    BSP_RC522_WriteReg(MFRC_CommandReg, MFRC_RESETPHASE);
    Sys_Delay(5); //保证完成复位

    //配置ISO14443A协议的工作模式
    BSP_RC522_WriteReg(MFRC_ModeReg, 0x3D); //设置CRC初始值为0x6363
    BSP_RC522_WriteReg(MFRC_TReloadRegL, 30); //设置定时器重装值低字节为0x1E
    BSP_RC522_WriteReg(MFRC_TReloadRegH, 0);
    BSP_RC522_WriteReg(MFRC_TModeReg, 0x8D);      // 定时器自动启动模式与高位预分频
    //     - MFRC522时钟 = 13.56MHz
    // - 定时器时钟 = 13.56MHz ÷ (318+1) ≈ 42.5kHz
    // - 重装值 = 30
    // - 一次超时 = 30 ÷ 42.5kHz ≈ 706μs
    BSP_RC522_WriteReg(MFRC_TPrescalerReg, 0x3E); // 定时器低位预分频
    BSP_RC522_WriteReg(MFRC_TxASKReg, 0x40);       //自动唤醒

    /* 4. 重启射频天线 */
    PCD_AntennaOff();
    Sys_Delay(1);
    PCD_AntennaOn();

}



/*******************************************************
 Author: PENG		Version: V1.0		Date:2025/06/26
 Function:			PCD_Request
 Description:		寻卡
 Calls:
 Called By:
 Input:				RequestMode: 讯卡方式
 	 	 	 	 	PICC_REQIDL: 寻天线区内未进入休眠状态
 	 	 	 	 	PICC_REQALL: 寻天线区内全部卡
 	 	 	 	 	pCardType: 用于保存卡片类型
 Output:			pCardType: 卡片类型
 	 	 	 	 	0x4400: Mifare_UltraLight
 	 	 	 	 	0x0400: Mifare_One(S50)
 	 	 	 	 	0x0200: Mifare_One(S70)
 	 	 	 	 	0x0800: Mifare_Pro(X)
 	 	 	 	 	0x4403: Mifare_DESFire
 Return:			status: 错误代码(PCD_OK、PCD_NOTAGERR、PCD_ERR)
 Others:
*******************************************************/
char PCD_Request(uint8_t RequestMode, uint8_t *pCardType)
{
	char status;
    uint16_t unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    RC522_ClrBitMask(MFRC_Status2Reg, 0x08);		//关内部温度传感器
    BSP_RC522_WriteReg(MFRC_BitFramingReg, 0x07);	//存储模式，发送模式，是否启动发送等
    RC522_SetBitMask(MFRC_TxControlReg, 0x03);	//配置调制信号13.56MHZ

    CmdFrameBuf[0] = RequestMode; //两种寻卡命令 一直为0x26 另一种为0x52 （所有卡）
    //旋转发送并接收的命令 
    //根据requestmode发送寻卡命令，寻卡成功后返回卡片类型
    //发送一个字节
    //接收的缓冲区是CmdFrameBuf，长度是unLen


    status = RC522_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 1, CmdFrameBuf, &unLen);

    if((status == PCD_OK) && (unLen == 0x10))
    {
        *pCardType = CmdFrameBuf[0];
        *(pCardType+1) = CmdFrameBuf[1];
    }
    else
    {
        status = PCD_ERR;
    }

    return status;
}



/*******************************************************
 Author: PAN        Version: V1.0       Date:2026/08/14
 Function:          PCD_Anticoll
 Description:       防冲突,获取卡号（UID）
                    ISO14443A协议规定：卡片UID为4字节
                    防冲突命令：0x93 + 0x20
                    卡片返回：4字节UID + 1字节BCC校验
 Calls:             RC522_CmdFrame
 Called By:         上层应用
 Input:             pSnr: 用于保存卡片序列号,4字节
 Output:            pSnr: 卡片序列号,4字节
 Return:            status: 错误代码(MFRC_OK、MFRC_NOTAGERR、MFRC_ERR)
 Others:            BCC校验 = UID[0] ^ UID[1] ^ UID[2] ^ UID[3]
*******************************************************/
char PCD_Anticoll(uint8_t *pSnr)
{
    char status;
    uint8_t i, snr_check = 0;
    uint16_t unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    RC522_ClrBitMask(MFRC_Status2Reg, 0x08);         // 关内部温度传感器
    BSP_RC522_WriteReg(MFRC_BitFramingReg, 0x00);   // 设置帧格式
    RC522_ClrBitMask(MFRC_CollReg, 0x80);            // 清除冲突检测位

    // ISO14443A 防冲突命令
    CmdFrameBuf[0] = PICC_ANTICOLL1;  // 0x93：防冲突命令第一层
    CmdFrameBuf[1] = 0x20;             // NVB：发送2字节完整字节

    status = RC522_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 2, CmdFrameBuf, &unLen);

    if(status == MFRC_OK)
    {
        // 复制4字节UID
        for(i=0; i<4; i++)
        {
            *(pSnr+i) = CmdFrameBuf[i];
            snr_check ^= CmdFrameBuf[i];  // 计算XOR校验
        }
        // 验证BCC校验（第5字节）
        if(snr_check != CmdFrameBuf[4])
        {
            status = MFRC_ERR;  // 校验失败
        }
    }

    RC522_SetBitMask(MFRC_CollReg, 0x80);  // 恢复冲突检测位
    return status;
}



/*******************************************************
 Author: PAN        Version: V1.0       Date:2026/08/14
 Function:          PCD_Select
 Description:       选卡
                    ISO14443A选卡命令：0x93 + 0x70 + UID(4字节) + BCC + CRC(2字节)
                    卡片返回：SAK（3字节）
 Calls:             RC522_CmdFrame、RC522_CalulateCRC
 Called By:         上层应用
 Input:             pSnr：卡片序列号,4字节（由PCD_Anticoll获取）
 Output:            无
 Return:            status: 错误代码(MFRC_OK、MFRC_ERR)
 Others:            SAK = Select Acknowledge，选卡应答
*******************************************************/
char PCD_Select(uint8_t *pSnr)
{
    char status;
    uint8_t i;
    uint16_t unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    // ISO14443A 选卡命令帧
    CmdFrameBuf[0] = PICC_ANTICOLL1;  // 0x93：防冲突命令第一层
    CmdFrameBuf[1] = 0x70;             // NVB：发送7字节完整字节
    CmdFrameBuf[6] = 0;                // BCC初始化

    // 填充4字节UID并计算BCC
    for(i=0; i<4; i++)
    {
        CmdFrameBuf[i+2] = *(pSnr + i);  // UID[0-3]
        CmdFrameBuf[6]  ^= *(pSnr + i);  // 计算BCC = UID[0]^UID[1]^UID[2]^UID[3]
    }

    // 计算CRC校验（2字节）
    RC522_CalulateCRC(CmdFrameBuf, 7, &CmdFrameBuf[7]);

    RC522_ClrBitMask(MFRC_Status2Reg, 0x08);  // 关内部温度传感器

    // 发送9字节选卡命令，接收SAK应答
    status = RC522_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 9, CmdFrameBuf, &unLen);

    // SAK应答为3字节（24位）
    if((status == MFRC_OK) && (unLen == 0x18))
    {
        status = MFRC_OK;  // 选卡成功
    }
    else
    {
        status = MFRC_ERR; // 选卡失败
    }

    return status;
}



/*******************************************************
 Author: PAN        Version: V1.0       Date:2026/08/14
 Function:          PCD_AuthState
 Description:       验证卡片密码
                    验证密码时,以扇区为单位,BlockAddr参数可以是同一个扇区的任意块
                    Mifare1卡片每个扇区有独立的密钥（KeyA和KeyB）
                    认证成功后，Status2Reg的BIT3(MFCryptoOn)会被置1
 Calls:             RC522_CmdFrame、BSP_RC522_ReadReg
 Called By:         上层应用
 Input:             AuthMode: 验证模式
                    PICC_AUTHENT1A (0x60): 验证A密钥
                    PICC_AUTHENT1B (0x61): 验证B密钥
                    BlockAddr: 块地址(0~63)
                    pKey: 密码(6字节)
                    pSnr: 卡片序列号,4字节（由PCD_Anticoll获取）
 Output:            无
 Return:            status: 错误代码(MFRC_OK、MFRC_ERR)
 Others:            认证命令帧: AuthMode + BlockAddr + Key(6字节) + UID(4字节) = 12字节
                    认证成功标志: Status2Reg BIT3 (MFCryptoOn) = 1
*******************************************************/
char PCD_AuthState(uint8_t AuthMode, uint8_t BlockAddr, uint8_t *pKey, uint8_t *pSnr)
{
    char status;
    uint16_t unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    // 构造认证命令帧
    CmdFrameBuf[0] = AuthMode;      // 认证模式（A或B密钥）  60是密钥A 61是密钥B
    CmdFrameBuf[1] = BlockAddr;     // 块地址 块8

    memcpy(&CmdFrameBuf[2], pKey, 6);   // 6字节密钥
    memcpy(&CmdFrameBuf[8], pSnr, 4);   // 4字节UID

    // 发送认证命令（MFRC_AUTHENT = 0x0E）
    status = RC522_CmdFrame(MFRC_AUTHENT, CmdFrameBuf, 12, CmdFrameBuf, &unLen);

    // 验证认证结果
    // 1. 检查命令是否执行成功 (status == MFRC_OK)
    // 2. 检查Status2Reg BIT3 (MFCryptoOn) 是否为1
    if((status != MFRC_OK) || (!(BSP_RC522_ReadReg(MFRC_Status2Reg) & 0x08)))
    {
        status = MFRC_ERR;  // 认证失败
    }

    return status;
}



/*******************************************************
 Author: PAN        Version: V1.0       Date:2026/08/14
 Function:          PCD_ReadBlock
 Description:       读MF1卡数据块
                    Mifare1卡片每个块16字节
                    块地址：0~63
                    前必须先认证（PCD_AuthState）
 Calls:             RC522_CmdFrame、RC522_CalulateCRC
 Called By:         上层应用
 Input:             BlockAddr: 块地址（0~63）
                    pData: 用于保存读出的数据,16字节
 Output:            pData: 读出的数据,16字节
 Return:            status: 错误代码(MFRC_OK、MFRC_ERR)
 Others:            读块命令帧: READ(0x30) + BlockAddr + CRC(2字节) = 4字节
                    卡片应答: 16字节数据 + 2字节CRC = 18字节 (0x90 = 144位)
*******************************************************/
char PCD_ReadBlock(uint8_t BlockAddr, uint8_t *pData)
{
    char status;
    uint16_t unLen;
    uint8_t i, CmdFrameBuf[MFRC_MAXRLEN];

    // 构造读块命令帧
    CmdFrameBuf[0] = PICC_READ;      // 0x30：读块命令 
    CmdFrameBuf[1] = BlockAddr;       // 块地址

    // 计算CRC校验（2字节）
    RC522_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);

    // 发送4字节命令，接收18字节应答
    status = RC522_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);

    // 验证应答（18字节 = 144位 = 0x90）
    if((status == MFRC_OK) && (unLen == 0x90))
    {
        // 复制16字节数据
        for(i=0; i<16; i++)
        {
            *(pData+i) = CmdFrameBuf[i];
        }
    }
    else
    {
        status = MFRC_ERR;  // 读取失败
    }

    return status;
}



/*******************************************************
 Author: PAN        Version: V1.0       Date:2026/08/16
 Function:          PCD_WriteBlock
 Description:       写MF1卡数据块
                    Mifare1卡片每个块16字节
                    写入前必须先认证（PCD_AuthState）
                    写操作分两步：1.发送写命令 2.发送数据

 写块流程：
 ┌────────────────────────────────────────────────────┐
 │ 1. 发送写命令                                      │
 │    命令: WRITE(0xA0) + BlockAddr + CRC             │
 │    长度: 4字节                                     │
 │    应答: ACK (0x0A, 4位)                           │
 │                                                    │
 │ 2. 发送数据                                        │
 │    数据: 16字节 + CRC                              │
 │    长度: 18字节                                    │
 │    应答: ACK (0x0A, 4位)                           │
 └────────────────────────────────────────────────────┘

 Calls:             RC522_CmdFrame、RC522_CalulateCRC
 Called By:         上层应用
 Input:             BlockAddr: 块地址（0~63）
                    pData: 待写入的数据,16字节
 Output:            无
 Return:            status: 错误代码(MFRC_OK、MFRC_ERR)
 Others:            ACK = 0x0A（4位应答，表示确认）
*******************************************************/
char PCD_WriteBlock(uint8_t BlockAddr, uint8_t *pData)
{
    char status;
    uint16_t unLen;
    uint8_t i, CmdFrameBuf[MFRC_MAXRLEN];

    /* Step 1: 发送写命令 */
    CmdFrameBuf[0] = PICC_WRITE;      // 0xA0：写块命令
    CmdFrameBuf[1] = BlockAddr;       // 块地址

    // 计算CRC校验
    RC522_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);

    // 发送4字节命令
    status = RC522_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);

    // 验证应答：ACK = 0x0A（4位）
    // unLen == 4：应答4位
    // (CmdFrameBuf[0] & 0x0F) == 0x0A：低4位为0x0A表示ACK
    if((status != MFRC_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
    {
        status = MFRC_ERR;  // 写命令失败
    }

    /* Step 2: 发送数据 */
    if(status == MFRC_OK)
    {
        // 填充16字节数据
        for(i=0; i<16; i++)
        {
            CmdFrameBuf[i] = *(pData+i);
        }

        // 计算CRC校验（16字节数据 + 2字节CRC = 18字节）
        RC522_CalulateCRC(CmdFrameBuf, 16, &CmdFrameBuf[16]);

        // 发送18字节数据
        status = RC522_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 18, CmdFrameBuf, &unLen);

        // 验证应答：ACK = 0x0A（4位）
        if((status != MFRC_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
        {
            status = MFRC_ERR;  // 写数据失败
        }
    }

    return status;
}



/*******************************************************
 Author: PAN        Version: V1.0       Date:2026/08/16
 Function:          PCD_Halt
 Description:       命令卡片进入休眠状态
                    休眠后卡片不再响应寻卡命令
                    如需再次读取，需要重新寻卡（REQALL 0x52）

 休眠命令帧：
 ┌──────┬──────┬──────┬──────┐
 │ 0x50 │ 0x00 │ CRCL │ CRCH │
 ├──────┼──────┼──────┼──────┤
 │ HALT │  0   │ CRC校验 │
 └──────┴──────┴──────┴──────┘

 Calls:             RC522_CmdFrame、RC522_CalulateCRC
 Called By:         上层应用
 Input:             无
 Output:            无
 Return:            status: 错误代码(MFRC_OK、MFRC_ERR)
 Others:            休眠后卡片不响应REQA(0x26)，只响应WUPA(0x52)
*******************************************************/
char PCD_Halt(void)
{
    char status;
    uint16_t unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    // 构造休眠命令帧
    CmdFrameBuf[0] = PICC_HALT;      // 0x50：休眠命令
    CmdFrameBuf[1] = 0x00;            // 固定为0

    // 计算CRC校验
    RC522_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);

    // 发送4字节命令（不需要等待应答）
    status = RC522_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);

    return status;
}

