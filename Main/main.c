#include "generalDefines.h"
#include "stm32f3xx_hal.h"
#include "modEffect.h"
#include "modPowerState.h"
#include "modOperationalState.h"
#include "modDelay.h"
#include "modPowerElectronics.h"
#include "modHiAmp.h"
#include "modConfig.h"
#include "modStateOfCharge.h"
#include "driverSWStorageManager.h"
#include "modUART.h"
#include "mainDataTypes.h"
#include "modCAN.h"
#include "modHiAmp.h"

// This next define enables / disables the watchdog
#define AllowDebug

IWDG_HandleTypeDef handleIWDG;
modConfigGeneralConfigStructTypedef *generalConfig;
modStateOfChargeStructTypeDef       *generalStateOfCharge;
modPowerElectricsPackStateTypedef   packState;

void SystemClock_Config(void);
void mainWatchDogInitAndStart(void);
void mainWatchDogReset(void);
void Error_Handler(void);

int main(void) {			
  HAL_Init();
  SystemClock_Config();
	modPowerStateInit(P_STAT_SET);																						// Enable power supply to keep operational// 初始化电源状态机
	mainWatchDogInitAndStart();// 初始化并启动看门狗
	// 所有后续函数必须严格按照该顺序调用
	generalConfig            = modConfigInit();																// Tell EEPROM the needed size for ConfigStruct// 初始化配置结构体
	generalStateOfCharge     = modStateOfChargeInit(&packState,generalConfig);// Tell EEPROM the needed size for StatOfChargeStruct// 初始化状态结构体
	driverSWStorageManagerInit();																							// Initializes EEPROM Memory// 初始化EEPROM存储管理器
	modConfigStoreAndLoadDefaultConfig();																			// Store default config if needed -> load config from EEPROM// 存储默认配置或从EEPROM加载配置
	modStateOfChargeStoreAndLoadDefaultStateOfCharge();												// Determin SoC from cell voltage if needed -> load StateOfCharge from EEPROM// 确定电池状态或从EEPROM加载电池状态  
	// Until here
	
	modPowerStateSetConfigHandle(generalConfig);                              // Tell the power state what input method is used en power on mode.// 设置电源状态机的输入方法
	modCommandsInit(&packState,generalConfig);// 初始化命令结构体
	modUARTInit();																	  												// Will act on UART message requests// 初始化UART通信 
	modCANInit(&packState,generalConfig);																			// Will act on CAN message requests// 初始化CAN通信任务
	modEffectInit();																													// Controls the effects on LEDs + buzzer// 初始化效果结构体
	modEffectChangeState(STAT_LED_DEBUG,STAT_FLASH);													// Set Debug LED to blinking mode// 设置调试LED为闪烁模式 
	modPowerElectronicsInit(&packState,generalConfig);												// 将测量所有电压并将其存入 packState// 初始化电源电子任务
	modOperationalStateInit(&packState,generalConfig,generalStateOfCharge);		// Will keep track of and control operational state (eg. normal use / charging / balancing / power down)// 初始化操作状态机
	modHiAmpInit(&packState,generalConfig);																		// Initialize the HiAmp shield enviroment if any// 初始化HiAmp环境  
	
  while(true) {
		modEffectTask();  // 控制LED和蜂鸣器的效果
		modPowerStateTask();// 处理电源状态
		modOperationalStateTask();// 处理操作状态
		modUARTTask();// 处理UART通信任务
		modCANTask();// 处理CAN通信任务   
		modHiAmpTask();// 处理HiAmp任务             
		mainWatchDogReset();// 重置看门狗 
		
		if(modPowerElectronicsTask())																						// 处理电源电子任务
			modStateOfChargeProcess();																						// 如果有新数据，处理SoC估计
  }
}

/* System Clock Configuration */
void SystemClock_Config(void) {// 配置系统时钟
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    Error_Handler();

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_I2C2|RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
  PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    Error_Handler();

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* IWDG init function */
void mainWatchDogInitAndStart(void) {// 初始化并启动看门狗
#ifndef AllowDebug  
  handleIWDG.Instance = IWDG;
  handleIWDG.Init.Prescaler = IWDG_PRESCALER_256;
  handleIWDG.Init.Window = 4095;
  handleIWDG.Init.Reload = 4095;
  if (HAL_IWDG_Init(&handleIWDG) != HAL_OK) {
    Error_Handler();
  }
	
  if(HAL_IWDG_Start(&handleIWDG) != HAL_OK) {
    Error_Handler();
  }
#endif
}

void mainWatchDogReset(void) {// 重置看门狗
#ifndef AllowDebug
	HAL_IWDG_Refresh(&handleIWDG);
#endif
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void){// 错误处理函数
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT// 开启断言检查

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif
