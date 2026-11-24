#include "driverSWStorageManager.h"
#include "generalDefines.h"
#include "mainDataTypes.h"
#include "modCAN.h"
#include "modConfig.h"
#include "modDelay.h"
#include "modEffect.h"
#include "modHiAmp.h"
#include "modOperationalState.h"
#include "modPowerElectronics.h"
#include "modPowerState.h"
#include "modStateOfCharge.h"
#include "modUART.h"
#include "stm32f3xx_hal.h"

// 下面的宏定义用于使能/禁用看门狗（注释掉则开启看门狗）
// #define AllowDebug

IWDG_HandleTypeDef handleIWDG;                       // 独立看门狗句柄
modConfigGeneralConfigStructTypedef* generalConfig;  // 全局配置结构体指针
modStateOfChargeStructTypeDef* generalStateOfCharge; // 全局电量状态(SoC)结构体指针
modPowerElectricsPackStateTypedef packState;         // 电池组电源状态

void SystemClock_Config(void);       // 系统时钟配置函数
void mainWatchDogInitAndStart(void); // 看门狗初始化并启动函数
void mainWatchDogReset(void);        // 看门狗喂狗(复位)函数
void Error_Handler(void);            // 错误处理函数

int main(void)
{
    HAL_Init();                    // 初始化HAL库
    SystemClock_Config();          // 配置系统时钟
    modPowerStateInit(P_STAT_SET); // 使能电源保持系统运行 / 初始化电源状态
    mainWatchDogInitAndStart();    // 初始化并启动看门狗
    // 以下各功能模块必须严格按照此顺序初始化（存在依赖关系）
    generalConfig = modConfigInit();                                        // 初始化配置结构体(告知EEPROM配置结构体所需大小)
    generalStateOfCharge = modStateOfChargeInit(&packState, generalConfig); // 初始化电量状态结构体(告知EEPROM该结构体所需大小)
    driverSWStorageManagerInit();                                           // 初始化EEPROM存储管理器
    modConfigStoreAndLoadDefaultConfig();                                   // 如需则存储默认配置 -> 从EEPROM加载配置
    modStateOfChargeStoreAndLoadDefaultStateOfCharge();                     // 如需则按电芯电压计算SoC -> 从EEPROM加载SoC
// 每次开机时，都会在RAM中强制设置安全的下电参数，与EEPROM无关。
// PC上位机工具会用旧的存储值(1.00A / 1000ms)重写整个配置结构体；
// 在377欧姆负载(~0.1A)下，会让电池组在开机1秒后立即下电。
// 之前带条件判断的补丁在阈值 <= 0.1A 时会跳过修复，
// 导致错误的1000ms超时仍然生效。现在只改RAM，避免与上位机工具争夺EEPROM。
#if 1
    generalConfig->notUsedCurrentThreshold = 0.05f; // 空闲电流阈值设为0.05A
    generalConfig->notUsedTimeout = 60 * 60 * 1000; // 空闲下电超时设为1小时
    // 每次开机写回一次，使上位机工具也能读到修正后的值，
    // 而不是会导致立即下电的旧值(1.00A / 1000ms)。
    modConfigStoreConfig();
#endif
// ---- LFP(磷酸铁锂)电芯电压阈值 - RAM覆盖 ----
// 上位机工具只在逐字段应用时才写入RAM；如果不写回EEPROM，
// 重启后这些值会丢失，芯片会回落到默认的锂离子限值(4.25/4.15V)。
// 因此每次开机强制写入正确的LFP限值。
// 仅修改RAM，此处不写入EEPROM(之前写回操作曾损坏已存储的SoC)。
#if 1
    generalConfig->cellHardUnderVoltage = 2.55f;   // 电芯硬欠压阈值
    generalConfig->cellHardOverVoltage = 3.65f;    // 电芯硬过压阈值
    generalConfig->cellLCSoftUnderVoltage = 2.88f; // 电芯低组软欠压阈值
    generalConfig->cellHCSoftUnderVoltage = 3.00f; // 电芯高组软欠压阈值
    generalConfig->cellSoftOverVoltage = 3.55f;    // 电芯软过压阈值
#endif
// -----------------------------------------------------------
// ---- 修改版电路板的分流电阻系数覆盖 ----
// R11 当前由 16 个 10 欧姆电阻并联 = 0.625 欧姆
// (原设计: 1 毫欧, 系数 -0.004494)
// 所需系数 = 5e-6 / R = 5e-6 / 0.625 = 0.000008
// 仅覆盖RAM；EEPROM仍保存标准值 -0.004494
// 如需恢复标准的 1 毫欧分流电阻：把 "#if 1" 改成 "#if 0"
#if 1
    generalConfig->shuntLCFactor = -0.000008f; // 分流电阻电流采样系数
#endif
// -------------------------------------------------------
// ---- 为阻性测试负载降低预充电阈值 ----
// 预充电MOS通过约200欧姆电阻给负载供电。在377欧姆负载下
// 输出电压被分压到约26V(40V电池组的65%)，永远达不到默认的
// 80%阈值 -> 进入ERROR_PRECHARGE循环(输出在26V <-> 0V间来回切换)。
// 将阈值降低到50%并给出2秒超时。
// 仅覆盖RAM；EEPROM保留默认的 0.80/300。恢复方法: "#if 1" -> "#if 0"
#if 1
    generalConfig->minimalPrechargePercentage = 0.35f; // 470欧姆预充电阻+377欧姆负载分压到约17.5V(44%)；45%阈值无法触达 -> 卡在预充电状态
    generalConfig->timeoutLCPreCharge = 2000;          // 低压预充电超时设为2000ms
#endif
    // -------------------------------------------------------
    // 以上所有覆盖仅到此处为止

    modPowerStateSetConfigHandle(generalConfig);                              // 将配置句柄传给电源状态机，设置输入方式与上电模式
    modCommandsInit(&packState, generalConfig);                               // 初始化命令结构体
    modUARTInit();                                                            // 初始化UART通信(响应UART消息请求)
    modCANInit(&packState, generalConfig);                                    // 初始化CAN通信(响应CAN消息请求)
    modEffectInit();                                                          // 初始化LED与蜂鸣器效果
    modEffectChangeState(STAT_LED_DEBUG, STAT_FLASH);                         // 设置调试LED为闪烁模式
    modPowerElectronicsInit(&packState, generalConfig);                       // 初始化电源电子(采集电压/电流等填充packState)
    modOperationalStateInit(&packState, generalConfig, generalStateOfCharge); // 初始化运行状态机(正常使用/充电/均衡/下电等)
    modHiAmpInit(&packState, generalConfig);                                  // 如存在HiAmp扩展板则初始化其环境

    // ================================================================
    // 安全释放 PC13 ---- 根本原因修复
    // PC13驱动SAFETY网络 -> Q12/Q13(BSS123)公共栅极。
    // 复位后默认是浮空输入；如果保持高电平，Q12/Q13导通，
    // 将DischargeEnable(PB10) / ChargeEnable(PB0)硬拉至GND，
    // 主MOS永远无法导通 -> 输出保持0V / 仅预充电约17.5V。
    // MCU必须主动将SAFETY拉低，才能解除硬件失效保护。
    // 放在所有模块初始化之后，确保后续GPIO初始化不会覆盖它。
    // ================================================================
    {
        GPIO_InitTypeDef gpioInit;
        RCC->AHBENR |= RCC_AHBENR_GPIOCEN; // 使能GPIOC时钟
        gpioInit.Pin = GPIO_PIN_13;
        gpioInit.Mode = GPIO_MODE_OUTPUT_PP; // 推挽输出
        gpioInit.Pull = GPIO_NOPULL;         // 不使用上下拉
        gpioInit.Speed = GPIO_SPEED_HIGH;    // 高速
        HAL_GPIO_Init(GPIOC, &gpioInit);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // SAFETY拉低 -> Q12/Q13截止
    }

    while (true)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // 主循环中持续将SAFETY保持低电平 -> Q12/Q13保持截止
        modEffectTask();                                       // 运行LED与蜂鸣器效果任务
        modPowerStateTask();                                   // 运行电源状态机任务
        modOperationalStateTask();                             // 运行运行状态机任务
        modUARTTask();                                         // 运行UART通信任务
        modCANTask();                                          // 运行CAN通信任务
        modHiAmpTask();                                        // 运行HiAmp扩展板任务
        mainWatchDogReset();                                   // 喂狗，复位看门狗计数

        if (modPowerElectronicsTask()) // 运行电源电子任务，返回真表示采样周期已到
            modStateOfChargeProcess(); // 处理电池数据，计算SoC
    }
}

/* 系统时钟配置 */
void SystemClock_Config(void)
{ // 配置系统时钟
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInit;

    // 选用HSI、LSI、HSE振荡源
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;                // 使能外部高速晶振HSE
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1; // HSE预分频1分频
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;                // 使能内部高速晶振HSI
    RCC_OscInitStruct.HSICalibrationValue = 16;             // HSI校准值16
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;                // 使能内部低速晶振LSI(供看门狗使用)
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;            // 使能PLL锁相环
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;    // PLL时钟源选择HSE
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;            // PLL倍频系数9 -> 72MHz系统时钟
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)    // 若振荡配置失败
        Error_Handler();

    // 配置各总线时钟分频
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;               // 系统时钟源选择PLL输出
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                      // AHB不分频
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;                       // APB1二分频
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;                       // APB2不分频
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) // 配置时钟并设置Flash等待周期2
        Error_Handler();

    // 配置外设时钟源：USART2、I2C1、I2C2、ADC12
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2 | RCC_PERIPHCLK_I2C1 | RCC_PERIPHCLK_I2C2 | RCC_PERIPHCLK_ADC12;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1; // USART2时钟源为PCLK1
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;       // ADC12时钟源为PLL输出1分频
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;    // I2C1时钟源为系统时钟
    PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_SYSCLK;    // I2C2时钟源为系统时钟
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)        // 若外设时钟配置失败
        Error_Handler();

    // 配置SysTick定时器为1ms中断(供HAL_Delay使用)
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

    // SysTick时钟源选择HCLK
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn 中断优先级配置 */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* IWDG 独立看门狗初始化函数 */
void mainWatchDogInitAndStart(void)
{ // 初始化并启动看门狗
#ifndef AllowDebug
    handleIWDG.Instance = IWDG;
    handleIWDG.Init.Prescaler = IWDG_PRESCALER_256; // 预分频256
    handleIWDG.Init.Window = 4095;                  // 窗口值
    handleIWDG.Init.Reload = 4095;                  // 重装载值(超时约1.09秒)
    if (HAL_IWDG_Init(&handleIWDG) != HAL_OK)
    { // 若初始化失败
        Error_Handler();
    }

    if (HAL_IWDG_Start(&handleIWDG) != HAL_OK)
    { // 若启动失败
        Error_Handler();
    }
#endif
}

void mainWatchDogReset(void)
{ // 喂狗，复位看门狗计数
#ifndef AllowDebug
    HAL_IWDG_Refresh(&handleIWDG); // 刷新看门狗，防止系统复位
#endif
}

/**
 * @brief  发生错误时执行的函数
 * @param  None
 * @retval None
 */
void Error_Handler(void)
{ // 错误处理函数
    /* USER CODE BEGIN Error_Handler */
    /* 用户可在此添加自己的实现来报告HAL错误返回状态 */
    while (1) // 出错后死循环挂起，由看门狗复位系统
    {
    }
    /* USER CODE END Error_Handler */
}

#ifdef USE_FULL_ASSERT // 使能断言参数检查

/**
 * @brief 报告发生assert_param错误的源文件名与行号
 * @param file: 源文件名指针
 * @param line: assert_param错误所在行号
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* 用户可在此添加自己的实现来报告文件名与行号，
       例如：printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}

#endif
