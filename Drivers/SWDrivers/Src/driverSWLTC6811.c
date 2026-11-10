#include "driverSWLTC6811.h"

static const uint16_t crc15Table[256]= {
    0x0,   0xc599, 0xceab, 0xb32,  0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac,
    0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1,
    0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e,
    0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b,
    0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd,
    0x2544, 0x2be,  0xc727, 0xcc15, 0x98c,  0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c,
    0x3d6e, 0xf8f7, 0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x7c2,  0xc25b, 0xc969, 0xcf0,  0xdf0d,
    0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf,
    0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640,
    0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
    0x4a88, 0x8f11, 0x57c,  0xc0e5, 0xcbd7, 0xe4e,  0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b,
    0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921,
    0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070,
    0x85e9, 0xf84,  0xca1d, 0xc12f, 0x4b6,  0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a,
    0x3528, 0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2,
    0xe46b, 0xef59, 0x2ac0, 0xd3a,  0xc8a3, 0xc391, 0x608,  0xd5f5, 0x106c, 0x1b5e, 0xdec7,
    0x54aa, 0x9133, 0x9a01, 0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06,
    0xab9f, 0x7862, 0xbdfb, 0xb6c9, 0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80,
    0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a, 0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41,
    0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 0x2fbc, 0x846,  0xcddf, 0xc6ed, 0x374,
    0xd089, 0x1510, 0x1e22, 0xdbbb, 0xaf8,  0xcf61, 0xc453, 0x1ca,  0xd237, 0x17ae, 0x1c9c,
    0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b, 0x2d02, 0xa76f, 0x62f6,
    0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3, 0x585a, 0x8ba7,
    0x4e3e, 0x450c, 0x8095
};

uint8_t driverSWLTC6811TotalNumerOfICs = 0;
driverLTC6811ConfigStructTypedef driverSWLTC6811ConfigStruct;

uint16_t pec15_calc(uint8_t len, uint8_t *data) {
    uint16_t remainder, addr;
    remainder = 16;
    for (uint8_t i = 0; i < len; i++) {
        addr = ((remainder >> 7) ^ data[i]) & 0xff;
        remainder = (remainder << 8) ^ crc15Table[addr];
    }
    return remainder * 2;
}

void driverSWLTC6811WakeUp(uint8_t total_ic) {
    uint8_t dummy = 0xFF;
    for (uint8_t i = 0; i < total_ic; i++) {
        driverHWSPI1Write(&dummy, 1, LTC_CS_GPIO_Port, LTC_CS_Pin);
    }
}

void driverSWLTC6811SendCmd(uint8_t cmd0, uint8_t cmd1) {
    uint8_t cmd[4];
    uint16_t cmd_pec;
    cmd[0] = cmd0;
    cmd[1] = cmd1;
    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);
    driverSWLTC6811Write(cmd, 4);
}

void driverSWLTC6811Write(uint8_t *writeBytes, uint8_t writeLength) {
    driverHWSPI1Write(writeBytes, writeLength, LTC_CS_GPIO_Port, LTC_CS_Pin);
}

void driverSWLTC6811WriteRead(uint8_t *writeBytes, uint8_t writeLength,
                              uint8_t *readBytes, uint8_t readLength) {
    driverHWSPI1WriteRead(writeBytes, writeLength, readBytes, readLength,
                          LTC_CS_GPIO_Port, LTC_CS_Pin);
}

void driverSWLTC6811Init(driverLTC6811ConfigStructTypedef configStruct, uint8_t totalNumberOfLTCs) {
    driverSWLTC6811TotalNumerOfICs = totalNumberOfLTCs;
    driverSWLTC6811ConfigStruct = configStruct;

    driverHWSPI1Init(LTC_CS_GPIO_Port, LTC_CS_Pin);

    driverSWLTC6811WakeUp(driverSWLTC6811TotalNumerOfICs);

    driverSWLTC6811WriteConfig(driverSWLTC6811ConfigStruct);
}
// 重新初始化LTC6811
void driverSWLTC6811ReInit(void) {
    driverSWLTC6811WakeUp(driverSWLTC6811TotalNumerOfICs);// 唤醒所有LTC6811
    driverSWLTC6811WriteConfig(driverSWLTC6811ConfigStruct);// 写入新的配置寄存器值 
}
// 启动LTC6811电池电压转换
void driverSWLTC6811StartCellVoltageConversion(void) {
    uint8_t md_bits;
    md_bits = (driverSWLTC6811ConfigStruct.ADCOPT & 0x02) >> 1;// 获取ADC转换时间配置
    uint8_t cmd0 = md_bits + 0x02;
    md_bits = (driverSWLTC6811ConfigStruct.ADCOPT & 0x01) << 7;// 获取ADC转换模式配置
    uint8_t cmd1 = md_bits + 0x60 + 0;
    driverSWLTC6811SendCmd(cmd0, cmd1);
}

// 启动LTC6811温度电压转换
void driverSWLTC6811StartTemperatureVoltageConversion(void) {
    uint8_t md_bits;
    md_bits = (driverSWLTC6811ConfigStruct.ADCOPT & 0x02) >> 1;// 获取ADC转换时间配置
    uint8_t cmd0 = md_bits + 0x04;
    md_bits = (driverSWLTC6811ConfigStruct.ADCOPT & 0x01) << 7;// 获取ADC转换模式配置
    uint8_t cmd1 = md_bits + 0x60 + 0;
    driverSWLTC6811SendCmd(cmd0, cmd1);
}
// 重置LTC6811电池电压寄存器
void driverSWLTC6811ResetCellVoltageRegisters(void) {
    driverSWLTC6811SendCmd(0x07, 0x11);// 发送重置电池电压寄存器命令
}
// 读取LTC6811电池电压
bool driverSWLTC6811ReadCellVoltages(driverLTC6811CellsTypedef cellVoltages[12]) {
    const uint8_t total_ic = driverSWLTC6811TotalNumerOfICs;// 获取LTC6811芯片总数
    const uint8_t RDCV_CMDs[4] = {0x04, 0x06, 0x08, 0x0A};// 读取电池电压命令
    bool allValid = true;
    uint8_t cell_idx = 0;
    // 共4个电池电压寄存器，每个寄存器包含3个电池电压
    for (uint8_t reg = 0; reg < 4; reg++) {// 读取4个电池电压寄存器
        uint8_t cmd[4] = {0x00, RDCV_CMDs[reg], 0, 0};
        uint16_t cmd_pec = pec15_calc(2, cmd);// 计算命令CRC
        cmd[2] = (uint8_t)(cmd_pec >> 8);
        cmd[3] = (uint8_t)(cmd_pec);

        const uint8_t NUM_RX_BYT = 8;// 接收8字节数据
        uint8_t rx_buf[8];

        driverSWLTC6811WriteRead(cmd, 4, rx_buf, NUM_RX_BYT);// 发送命令并读取数据

        uint16_t calc_pec = pec15_calc(6, rx_buf);// 计算接收数据CRC
        uint16_t recv_pec = ((uint16_t)rx_buf[6] << 8) | rx_buf[7];// 获取接收数据CRC
        if (calc_pec != recv_pec) {// 校验接收数据CRC
            allValid = false;
        }

        uint16_t codes[3];// 存储3个电池电压原始值
        codes[0] = rx_buf[0] | ((uint16_t)rx_buf[1] << 8);// 获取第1个电池电压原始值
        codes[1] = rx_buf[2] | ((uint16_t)rx_buf[3] << 8);
        codes[2] = rx_buf[4] | ((uint16_t)rx_buf[5] << 8);

        for (uint8_t k = 0; k < 3 && cell_idx < 12; k++) {// 处理3个电池电压
            float v = (float)codes[k] * 0.0015f;// 将原始值转换为电压值
            if (v > 5.0f) {// 校验电压值是否超过5V
                allValid = false;
                v = 0.0f;
            }
            cellVoltages[cell_idx].cellVoltage = v;// 存储电池电压
            cellVoltages[cell_idx].cellNumber = cell_idx;// 存储电池编号
            cell_idx++;
        }
    }

    return allValid;
}
// 读取LTC6811温度电压
bool driverSWLTC6811ReadTempVoltages(uint16_t tempVoltages[3]) {
    uint8_t cmd[4] = {0x00, 0x0C, 0, 0};
    uint16_t cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);

    uint8_t rx_buf[8];
    driverSWLTC6811WriteRead(cmd, 4, rx_buf, 8);

    uint16_t calc_pec = pec15_calc(6, rx_buf);// 计算接收数据CRC
    uint16_t recv_pec = ((uint16_t)rx_buf[6] << 8) | rx_buf[7];// 获取接收数据CRC
    bool pec_ok = (calc_pec == recv_pec);// 校验接收数据CRC

    tempVoltages[0] = rx_buf[0] | ((uint16_t)rx_buf[1] << 8);// 获取第1个温度电压原始值
    tempVoltages[1] = rx_buf[2] | ((uint16_t)rx_buf[3] << 8);// 获取第2个温度电压原始值
    tempVoltages[2] = rx_buf[4] | ((uint16_t)rx_buf[5] << 8);// 获取第3个温度电压原始值

    return pec_ok;
}

void driverSWLTC6811WriteConfigRegisters(uint8_t total_ic, uint8_t config[][6]) {
    const uint8_t BYTES_IN_REG = 6;
    const uint8_t CMD_LEN = 4 + (8 * total_ic);
    uint8_t *cmd;
    uint16_t data_pec;
    uint16_t cmd_pec;
    uint8_t cmd_index;

    cmd = (uint8_t *)malloc(CMD_LEN * sizeof(uint8_t));
    cmd[0] = 0x00;
    cmd[1] = 0x01;
    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);
    cmd_index = 4;

    for (uint8_t current_ic = total_ic; current_ic > 0; current_ic--) {
        for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++) {
            cmd[cmd_index] = config[current_ic - 1][current_byte];
            cmd_index = cmd_index + 1;
        }
        data_pec = (uint16_t)pec15_calc(BYTES_IN_REG, &config[current_ic - 1][0]);
        cmd[cmd_index] = (uint8_t)(data_pec >> 8);
        cmd[cmd_index + 1] = (uint8_t)data_pec;
        cmd_index = cmd_index + 2;
    }

    driverSWLTC6811Write(cmd, CMD_LEN);
    free(cmd);
}

void driverSWLTC6811WriteConfig(driverLTC6811ConfigStructTypedef configStruct) {
    uint8_t cfgra[6];
    uint16_t uvRaw, ovRaw;

    uvRaw = (uint16_t)((configStruct.CellUnderVoltageLimit * 1000.0f) / 16.0f) - 1;
    ovRaw = (uint16_t)((configStruct.CellOverVoltageLimit * 1000.0f) / 16.0f);

    cfgra[0] = 0;
    if (configStruct.ADCOPT) cfgra[0] |= 0x01;
    if (configStruct.REFON)  cfgra[0] |= 0x04;
    for (int i = 0; i < 5; i++) {
        if (configStruct.GPIO[i]) cfgra[0] |= (0x01 << (i + 3));
    }

    cfgra[1] = (uint8_t)(uvRaw & 0x00FF);
    cfgra[2] = (uint8_t)(((uvRaw >> 8) & 0x000F) << 4) | (ovRaw & 0x000F);
    cfgra[3] = (uint8_t)((ovRaw >> 4) & 0x00FF);
    cfgra[4] = (uint8_t)(configStruct.DisChargeEnableMask & 0x00FF);
    cfgra[5] = (uint8_t)(configStruct.DisChargeEnableMask >> 8);

    uint8_t configPointer[1][6];
    memcpy(configPointer[0], cfgra, 6);

    driverSWLTC6811WriteConfigRegisters(driverSWLTC6811TotalNumerOfICs, configPointer);
}

bool driverSWLTC6811ReadConfigRegisters(uint8_t total_ic, uint8_t r_config[][8]) {
    const uint8_t BYTES_IN_REG = 8;
    uint8_t cmd[4] = {0x00, 0x02, 0, 0};
    uint16_t cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);

    uint8_t *rx_data = (uint8_t *)malloc((BYTES_IN_REG * total_ic) * sizeof(uint8_t));
    driverSWLTC6811WriteRead(cmd, 4, rx_data, BYTES_IN_REG * total_ic);

    bool pec_error = false;
    for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++) {
        for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++) {
            r_config[current_ic][current_byte] = rx_data[current_byte + (current_ic * BYTES_IN_REG)];
        }
        uint16_t received_pec = (r_config[current_ic][6] << 8) | r_config[current_ic][7];
        uint16_t data_pec = pec15_calc(6, &r_config[current_ic][0]);
        if (received_pec != data_pec) {
            pec_error = true;
        }
    }

    free(rx_data);
    return !pec_error;
}

// 读取LTC6811配置寄存器
bool driverSWLTC6811ReadConfig(driverLTC6811ConfigStructTypedef *configStruct) {
    uint8_t configPointer[1][8];
    bool returnval;

    returnval = driverSWLTC6811ReadConfigRegisters(driverSWLTC6811TotalNumerOfICs, configPointer);// 读取LTC6811配置寄存器

    configStruct->ADCOPT       = configPointer[0][0] & 0x01;//// 读取ADC选项位
    configStruct->REFON        = (configPointer[0][0] & 0x04) >> 2;// 读取参考电压使能位
    configStruct->GPIO[0]      = (configPointer[0][0] & 0x08) >> 3;// 读取GPIO引脚0
    configStruct->GPIO[1]      = (configPointer[0][0] & 0x10) >> 4;// 读取GPIO引脚1
    configStruct->GPIO[2]      = (configPointer[0][0] & 0x20) >> 5;// 读取GPIO引脚2
    configStruct->GPIO[3]      = (configPointer[0][0] & 0x40) >> 6;// 读取GPIO引脚3
    configStruct->GPIO[4]      = (configPointer[0][0] & 0x80) >> 7;// 读取GPIO引脚4

    uint16_t uvRaw = (uint16_t)configPointer[0][1] | ((uint16_t)(configPointer[0][2] & 0xF0) << 4);// 读取电池电压下限原始值
    uint16_t ovRaw = ((uint16_t)(configPointer[0][2] & 0x0F) << 4) | ((uint16_t)configPointer[0][3] << 4);// 读取电池电压上限原始值
    configStruct->CellUnderVoltageLimit = ((float)(uvRaw + 1) * 16.0f) / 1000.0f;// 读取电池电压下限
    configStruct->CellOverVoltageLimit  = ((float)ovRaw * 16.0f) / 1000.0f;// 读取电池电压上限

    configStruct->DisChargeEnableMask  = (uint16_t)configPointer[0][4] | ((uint16_t)configPointer[0][5] << 8);// 读取放电使能掩码   

    return returnval;
}

bool driverSWLTC6811ReadFlagRegisters(uint8_t total_ic, uint8_t flagRegisters[][8]) {
    const uint8_t BYTES_IN_REG = 8;
    uint8_t cmd[4] = {0x00, 0x10, 0, 0};
    uint16_t cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);

    uint8_t *rx_data = (uint8_t *)malloc((BYTES_IN_REG * total_ic) * sizeof(uint8_t));
    driverSWLTC6811WriteRead(cmd, 4, rx_data, BYTES_IN_REG * total_ic);

    bool pec_error = false;
    for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++) {
        for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++) {
            flagRegisters[current_ic][current_byte] = rx_data[current_byte + (current_ic * BYTES_IN_REG)];
        }
        uint16_t received_pec = (flagRegisters[current_ic][6] << 8) | flagRegisters[current_ic][7];
        uint16_t data_pec = pec15_calc(6, &flagRegisters[current_ic][0]);
        if (received_pec != data_pec) {
            pec_error = true;
        }
    }

    free(rx_data);
    return !pec_error;
}
// 读取LTC6811电池电压标志
bool driverSWLTC6811ReadVoltageFlags(uint16_t *underVoltageFlags, uint16_t *overVoltageFlags) {
    uint8_t statRegisters[1][8];
    bool returnVal;

    *underVoltageFlags = 0;
    *overVoltageFlags = 0;

    returnVal = driverSWLTC6811ReadFlagRegisters(1, statRegisters);// 读取标志寄存器

    uint8_t uv_bits = statRegisters[0][0];// 读取欠压标志位
    uint8_t ov_bits = statRegisters[0][1];// 读取过压标志位

    for (uint8_t cell = 0; cell < driverSWLTC6811ConfigStruct.noOfCells && cell < 8; cell++) {// 遍历8个电池
        if (uv_bits & (1 << cell)) {// 检查欠压标志位
            *underVoltageFlags |= (1 << cell);// 设置欠压标志位
        }
        if (ov_bits & (1 << cell)) {// 检查过压标志位
            *overVoltageFlags |= (1 << cell);// 设置过压标志位
        }
    }

    if (driverSWLTC6811ConfigStruct.noOfCells > 8) {// 检查电池数量是否大于8个
        uint8_t uv_bits2 = statRegisters[0][2] & 0x0F;// 读取欠压标志位2
        uint8_t ov_bits2 = statRegisters[0][3] & 0x0F;// 读取过压标志位2
        for (uint8_t cell = 0; cell < (driverSWLTC6811ConfigStruct.noOfCells - 8) && cell < 4; cell++) {// 遍历剩余电池
            if (uv_bits2 & (1 << cell)) {// 检查欠压标志位2
                *underVoltageFlags |= (1 << (cell + 8));// 设置欠压标志位2
            }
            if (ov_bits2 & (1 << cell)) {// 检查过压标志位2
                *overVoltageFlags |= (1 << (cell + 8));// 设置过压标志位2           
            }
        }
    }

    return returnVal;
}
// 使能LTC6811电池均衡电阻
void driverSWLTC6811EnableBalanceResistors(uint16_t balanceEnableMask) {
    driverSWLTC6811ConfigStruct.DisChargeEnableMask = balanceEnableMask;// 设置电池均衡电阻使能掩码
    driverSWLTC6811WriteConfig(driverSWLTC6811ConfigStruct);// 写入配置寄存器
}
// 外部温度转换函数
float driverSWLTC6811ConvertTemperatureExt(uint16_t inputValue, uint32_t ntcNominal,
                                           uint32_t ntcSeriesResistance, uint16_t ntcBetaFactor,
                                           float ntcNominalTemp) {
    static float scalar;
    static float steinhart;

    scalar = 4095.0f / (float)inputValue - 1.0f;
    scalar = (float)ntcSeriesResistance / scalar;
    steinhart = scalar / (float)ntcNominal;
    steinhart = log(steinhart);
    steinhart /= (float)ntcBetaFactor;
    steinhart += 1.0f / (ntcNominalTemp + 273.15f);
    steinhart = 1.0f / steinhart;
    steinhart -= 273.15f;

    if (steinhart < -30.0f)
        steinhart = 200;

    return steinhart;
}

float driverSWLTC6811ConvertTemperatureInt(uint16_t inputValue) {
    return (float)inputValue * 1.5f / 8.0f - 273.15f;
}