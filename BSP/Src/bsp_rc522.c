/*******************************************************
 * RC522 专用 BSP 实现
 *******************************************************/

#include "bsp_rc522.h"

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





void BSP_RC522_Init(void)
{
    uint8_t tx_control_value;

    /* 软复位 MFRC522，等待芯片复位完成 */
    BSP_RC522_WriteReg(MFRC_CommandReg, MFRC_RESETPHASE);
    HAL_Delay(50);

    /* ModeReg（0x11）：设置 CRC 初始值为 0x6363 */
    BSP_RC522_WriteReg(MFRC_ModeReg, 0x3D);

    /* TReloadRegL（0x2D）：设置定时器重装值低字节为 0x1E（30） */
    BSP_RC522_WriteReg(MFRC_TReloadRegL, 0x1E);

    /* TReloadRegH（0x2C）：设置定时器重装值高字节为 0x00 */
    BSP_RC522_WriteReg(MFRC_TReloadRegH, 0x00);

    /* TModeReg（0x2A）：设置定时器模式为自动重装 */
    BSP_RC522_WriteReg(MFRC_TModeReg, 0x8D);

    /* TPrescalerReg（0x2B）：设置定时器预分频值为 0x3E */
    BSP_RC522_WriteReg(MFRC_TPrescalerReg, 0x3E);

    /* TxASKReg（0x15）：设置 100% ASK 调制 */
    BSP_RC522_WriteReg(MFRC_TxASKReg, 0x40);

    /* 读取发送控制寄存器并关闭天线 */
    tx_control_value = BSP_RC522_ReadReg(MFRC_TxControlReg);//读取0x14寄存器的值
    BSP_RC522_WriteReg(MFRC_TxControlReg,
                       (uint8_t)(tx_control_value & (uint8_t)~MFRC_TX_ANTENNA_MASK));

    /* 再次读取发送控制寄存器并开启天线 */
    tx_control_value = BSP_RC522_ReadReg(MFRC_TxControlReg);//读取0x14寄存器的值
    BSP_RC522_WriteReg(MFRC_TxControlReg,
                       (uint8_t)(tx_control_value | MFRC_TX_ANTENNA_MASK));
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
    // 清除 BIT0 和 BIT1，关闭射频输出
    BSP_RC522_ClearRegisterBit(MFRC_TxControlReg, 0x03); 
}

void RFID_RC522_Init(void)
{
    //软件传输复位功能  命令寄存器
    BSP_RC522_WriteReg(MFRC_CommandReg, MFRC_RESETPHASE);
    sys_delay_ms(5); //保证完成复位

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
    sys_delay_ms(1);
    PCD_AntennaOn();

}
