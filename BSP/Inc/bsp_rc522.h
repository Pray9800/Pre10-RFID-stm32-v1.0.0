/*******************************************************
 * RC522 专用 BSP 接口
 *******************************************************/

#ifndef __BSP_RC522_H
#define __BSP_RC522_H

#include "bsp_i2c.h"





/* RC522 I2C 7 位地址 0x28，转换为 HAL 库使用的 8 位地址 0x50 */
#define RC522_I2C_ADDR         (0x28 << 1)


/* MFRC522 寄存器定义 */
// PAGE 0: Command and Status
#define MFRC_RFU00                  0x00
#define MFRC_CommandReg             0x01
#define MFRC_ComIEnReg              0x02
#define MFRC_DivlEnReg              0x03
#define MFRC_ComIrqReg              0x04
#define MFRC_DivIrqReg              0x05
#define MFRC_ErrorReg               0x06
#define MFRC_Status1Reg             0x07
#define MFRC_Status2Reg             0x08
#define MFRC_FIFODataReg            0x09
#define MFRC_FIFOLevelReg           0x0A
#define MFRC_WaterLevelReg          0x0B
#define MFRC_ControlReg             0x0C
#define MFRC_BitFramingReg          0x0D
#define MFRC_CollReg                0x0E
#define MFRC_RFU0F                  0x0F

// PAGE 1: Communication
#define MFRC_RFU10                  0x10
#define MFRC_ModeReg                0x11
#define MFRC_TxModeReg              0x12
#define MFRC_RxModeReg              0x13
#define MFRC_TxControlReg           0x14
#define MFRC_TxASKReg               0x15    // 官方规范名称: TxASKReg中文手册有误
#define MFRC_TxSelReg               0x16
#define MFRC_RxSelReg               0x17
#define MFRC_RxThresholdReg         0x18
#define MFRC_DemodReg               0x19
#define MFRC_RFU1A                  0x1A
#define MFRC_RFU1B                  0x1B
#define MFRC_MfTxReg                0x1C
#define MFRC_MfRxReg                0x1D
#define MFRC_RFU1E                  0x1E
#define MFRC_SerialSpeedReg         0x1F

// PAGE 2: Configuration
#define MFRC_RFU20                  0x20
#define MFRC_CRCResultRegM          0x21
#define MFRC_CRCResultRegL          0x22
#define MFRC_RFU23                  0x23
#define MFRC_ModWidthReg            0x24
#define MFRC_RFU25                  0x25
#define MFRC_RFCfgReg               0x26
#define MFRC_GsNReg                 0x27
#define MFRC_CWGsPReg               0x28    // 官方规范名称: CWGsPReg 原来名称MFRC_CWGsCfgReg
#define MFRC_ModGsPReg              0x29    // 官方规范名称: ModGsPReg 原来名称MFRC_ModGsCfgReg
#define MFRC_TModeReg               0x2A
#define MFRC_TPrescalerReg          0x2B
#define MFRC_TReloadRegH            0x2C
#define MFRC_TReloadRegL            0x2D
#define MFRC_TCounterValueRegH      0x2E
#define MFRC_TCounterValueRegL      0x2F

// PAGE 3: Test Register
#define MFRC_RFU30                  0x30
#define MFRC_TestSel1Reg            0x31
#define MFRC_TestSel2Reg            0x32
#define MFRC_TestPinEnReg           0x33
#define MFRC_TestPinValueReg        0x34
#define MFRC_TestBusReg             0x35
#define MFRC_AutoTestReg            0x36
#define MFRC_VersionReg             0x37
#define MFRC_AnalogTestReg          0x38
#define MFRC_TestDAC1Reg            0x39
#define MFRC_TestDAC2Reg            0x3A
#define MFRC_TestADCReg             0x3B
#define MFRC_RFU3C                  0x3C
#define MFRC_RFU3D                  0x3D
#define MFRC_RFU3E                  0x3E
#define MFRC_RFU3F                  0x3F

/* 缓冲区与帧长定义 */
#define MFRC_FIFO_LENGTH            64
#define MFRC_MAXRLEN                18

/* MFRC522 指令集 */
#define MFRC_IDLE                   0x00    // 取消当前命令执行
#define MFRC_MEM                    0x01    // 存25字节到内部缓冲区
#define MFRC_GEN_RANDOMID           0x02    // 生成10字节随机ID
#define MFRC_CALCCRC                0x03    // 激活CRC计算
#define MFRC_TRANSMIT               0x04    // 发送FIFO缓冲区内容
#define MFRC_NOCMDCHANGE            0x07    // 无命令改变
#define MFRC_RECEIVE                0x08    // 激活接收器接收数据
#define MFRC_TRANSCEIVE             0x0C    // 发送并接收数据
#define MFRC_AUTHENT                0x0E    // 执行Mifare密钥认证
#define MFRC_RESETPHASE             0x0F    // 软复位MFRC522

/* MFRC522 天线控制位 */
#define MFRC_TX_ANTENNA_MASK        0x03

/* 函数状态返回值定义 */
#define MFRC_OK                     (char)0
#define MFRC_NOTAGERR               (char)(-1)
#define MFRC_ERR                    (char)(-2)

/* CommIrqReg 位掩码定义 */
/* Rx interrupt (RxIRq) - 数据接收完成 / 寻卡完成标志，位5 */
#define MFRC_RxIRq                  (0x20U)
/* 写入 CommIrqReg 清除中断标志的掩码（只保留保守位） */
#define MFRC_CommIrqClear           (0x7FU)





/* RC522 IRQ 中断引脚定义 */
#define RC522_IRQ_PIN          GPIO_PIN_0
#define RC522_IRQ_PORT         GPIOB

uint8_t BSP_RC522_IsDeviceReady(void);
HAL_StatusTypeDef BSP_RC522_WriteReg(uint8_t reg_addr, uint8_t value);
uint8_t BSP_RC522_ReadReg(uint8_t reg_addr);
HAL_StatusTypeDef BSP_RC522_WriteBuffer(uint8_t reg_addr, uint8_t *pData, uint16_t size);
HAL_StatusTypeDef BSP_RC522_ReadBuffer(uint8_t reg_addr, uint8_t *pData, uint16_t size);
uint8_t BSP_RC522_ReadIRQPin(void);
void BSP_RC522_Init(void);

#endif /* __BSP_RC522_H */