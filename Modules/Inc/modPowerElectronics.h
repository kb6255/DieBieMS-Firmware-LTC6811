#ifndef __MODPOWERELECTRONICS_H
#define __MODPOWERELECTRONICS_H

#include "driverSWISL28022.h"
#include "driverHWADC.h"
#include "driverSWLTC6811.h"
#include "driverHWSwitches.h"
#include "driverSWEMC2305.h"
#include "modDelay.h"
#include "modConfig.h"
#include "modPowerState.h"
#include "stdbool.h"
#include "math.h"

#define NoOfCellsPossibleOnChip	12
#define NoOfTempSensors         13
#define PRECHARGE_PERCENTAGE 		0.75f
#define TotalLTCICs							1
#define ISLErrorThreshold       10

typedef enum {
	TEMP_EXT_LTC_NTC0 = 0,									// EXT on master BMS on LTC
	TEMP_EXT_LTC_NTC1,											// EXT on master BMS on LTC
	TEMP_INT_LTC_CHIP,											// Int on master BMS inside LTC Chip
	TEMP_INT_STM_NTC,												// Int on master BMS outside STM Chip
	TEMP_EXT_ADC_NTC0,											// Ext on slave BMS
	TEMP_EXT_ADC_NTC1,											// Ext on slave BMS
	TEMP_EXT_ADC_NTC2,											// Ext on slave BMS
	TEMP_EXT_ADC_NTC3,											// Ext on slave BMS
	TEMP_EXT_ADC_NTC4,											// Ext on slave BMS
	TEMP_EXT_ADC_NTC5,											// Ext on slave BMS
	TEMP_INT_ADC_NTC6,											// Int on slave BMS
	TEMP_INT_ADC_NTC7,											// Int on slave BMS
	TEMP_INT_SHT														// Int on slave BMS
} modPowerElectronicsTemperatureSensorMapping;

typedef enum {
	PACK_STATE_ERROR_HARD_CELLVOLTAGE = 0,
	PACK_STATE_ERROR_SOFT_CELLVOLTAGE,
	PACK_STATE_ERROR_OVER_CURRENT,
	PACK_STATE_NORMAL,
} modPowerElectronicsPackOperationalCellStatesTypedef;

typedef struct {
	// Master BMS
	uint8_t  throttleDutyCharge;// 充电占空比
	uint8_t  throttleDutyDischarge;// 放电占空比
	float    SoC;// 电池荷电状态
	float    SoCCapacityAh;// 电池荷电容量Ah
	OperationalStateTypedef operationalState;// 操作状态
	float    packVoltage;// 电池电压
	float    packCurrent;// 电池电流
	float    packPower;// 电池功率
	float    loCurrentLoadCurrent;// 低电流负载电流
	float    loCurrentLoadVoltage;// 低电流负载电压
	float    cellVoltageHigh;// 电池电压高
	float    cellVoltageLow;// 电池电压低
	float    cellVoltageAverage;// 电池电压平均
	float    cellVoltageMisMatch;// 电池电压不匹配
	uint16_t cellBalanceResistorEnableMask;// 电池平衡电阻使能掩码
	float    temperatures[NoOfTempSensors];// 温度传感器温度
	float    tempBatteryHigh;// 电池温度高
	float    tempBatteryLow;// 电池温度低
	float    tempBatteryAverage;// 电池温度平均
	float    tempBMSHigh;// BMS温度高
	float    tempBMSLow;// BMS温度低
	float    tempBMSAverage;// BMS温度平均
	uint8_t  preChargeDesired;// 预充电需求
	uint8_t  disChargeDesired;// 放电需求
	uint8_t  disChargeLCAllowed;// 放电LC允许
	uint8_t  disChargeHCAllowed;// 放电HC允许
	uint8_t  chargeDesired;// 充电需求
	uint8_t  chargeAllowed;// 充电允许
	uint8_t  safetyOverCANHCSafeNSafe;// CAN HC安全N安全
	uint8_t  chargeCurrentDetected;// 充电电流检测
	uint8_t  chargeBalanceActive;// 充电均衡激活
	uint8_t  powerButtonActuated;// 电源按钮操作
	uint8_t  packInSOA;// 电池在SOA内
	uint8_t  watchDogTime;// 看门狗时间
	driverLTC6811CellsTypedef cellVoltagesIndividual[NoOfCellsPossibleOnChip];// 电池电压
	modPowerElectronicsPackOperationalCellStatesTypedef packOperationalCellState;// 电池操作状态0000
	
	// Slave BMS
	uint8_t  hiAmpShieldPresent;// 高电流负载屏蔽
	float    hiCurrentLoadVoltage;// 高电流负载电压
	float    hiCurrentLoadCurrent;// 高电流负载电流
	float		 hiCurrentLoadPower;// 高电流负载功率
	float    auxVoltage;// 辅助电压
	float    auxCurrent;// 辅助电流
	float		 auxPower;// 辅助功率
	uint8_t  aux0EnableDesired;// 辅助0使能需求
	uint8_t  aux0Enabled;// 辅助0使能
	uint8_t  aux0LoadIncorrect;// 辅助0负载错误
	uint8_t  aux1EnableDesired;// 辅助1使能需求
	uint8_t  aux1Enabled;// 辅助1使能
	uint8_t  aux1LoadIncorrect;// 辅助1负载错误
	uint8_t  auxDCDCEnabled;// 辅助DCDC使能
	uint8_t  auxDCDCOutputOK;// 辅助DCDC输出正常
	float    humidity;// 湿度
	uint8_t  hiLoadEnabled;// 高电流负载使能
	uint8_t  hiLoadPreChargeEnabled;// 高电流负载预充电使能
	uint8_t  hiLoadPreChargeError;// 高电流负载预充电错误
	uint8_t	 IOIN1;// IO输入1
	uint8_t  IOOUT0;// IO输出0
	uint8_t  FANSpeedDutyDesired;// 风扇转速占空比需求
	driverSWEMC2305FanStatusTypeDef FANStatus;// 风扇状态
} modPowerElectricsPackStateTypedef;

void modPowerElectronicsInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfig);
bool modPowerElectronicsTask(void);
void modPowerElectronicsAllowForcedOn(bool allowedState);
void modPowerElectronicsSetPreCharge(bool newState);
bool modPowerElectronicsSetDisCharge(bool newState);
void modPowerElectronicsSetCharge(bool newState);
void modPowerElectronicsDisableAll(void);
void modPowerElectronicsCalculateCellStats(void);
void modPowerElectronicsSubTaskBalaning(void);
void modPowerElectronicsSubTaskVoltageWatch(void);
void modPowerElectronicsUpdateSwitches(void);
void modPowerElectronicsSortCells(driverLTC6811CellsTypedef *cells, uint8_t cellCount);
void modPowerElectronicsCalcTempStats(void);
void modPowerElectronicsCalcThrottle(void);
int32_t modPowerElectronicsMapVariableInt(int32_t inputVariable, int32_t inputLowerLimit, int32_t inputUpperLimit, int32_t outputLowerLimit, int32_t outputUpperLimit);
float modPowerElectronicsMapVariableFloat(float inputVariable, float inputLowerLimit, float inputUpperLimit, float outputLowerLimit, float outputUpperLimit);
void modPowerElectronicsInitISL(void);
bool modPowerElectronicsHCSafetyCANAndPowerButtonCheck(void);
void modPowerElectronicsResetBalanceModeActiveTimeout(void);

#endif