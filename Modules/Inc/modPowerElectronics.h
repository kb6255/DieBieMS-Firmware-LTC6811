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
#define PRECHARGE_PERCENTAGE 		0.35f
#define TotalLTCICs							1  //��LTC IC����
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
	uint8_t  throttleDutyCharge;// ���ռ�ձ�
	uint8_t  throttleDutyDischarge;// �ŵ�ռ�ձ�
	float    SoC;// ��غɵ�״̬
	float    SoCCapacityAh;// ��غɵ�����Ah
	OperationalStateTypedef operationalState;// ����״̬
	float    packVoltage;// ��ص�ѹ
	float    packCurrent;// ��ص���
	float    packPower;// ��ع���
	float    loCurrentLoadCurrent;// �͵������ص���
	float    loCurrentLoadVoltage;// �͵������ص�ѹ
	float    cellVoltageHigh;// ��ص�ѹ��
	float    cellVoltageLow;// ��ص�ѹ��
	float    cellVoltageAverage;// ��ص�ѹƽ��
	float    cellVoltageMisMatch;// ��ص�ѹ��ƥ��
	uint16_t cellBalanceResistorEnableMask;// ���ƽ�����ʹ������
	float    temperatures[NoOfTempSensors];// �¶ȴ������¶�
	float    tempBatteryHigh;// ����¶ȸ�
	float    tempBatteryLow;// ����¶ȵ�
	float    tempBatteryAverage;// ����¶�ƽ��
	float    tempBMSHigh;// BMS�¶ȸ�
	float    tempBMSLow;// BMS�¶ȵ�
	float    tempBMSAverage;// BMS�¶�ƽ��
	uint8_t  preChargeDesired;// Ԥ�������
	uint8_t  disChargeDesired;// �ŵ�����
	uint8_t  disChargeLCAllowed;// �ŵ�LC����
	uint8_t  disChargeHCAllowed;// �ŵ�HC����
	uint8_t  chargeDesired;// �������
	uint8_t  chargeAllowed;// �������
	uint8_t  safetyOverCANHCSafeNSafe;// CAN HC��ȫN��ȫ
	uint8_t  chargeCurrentDetected;// ���������
	uint8_t  chargeBalanceActive;// �����⼤��
	uint8_t  powerButtonActuated;// ��Դ��ť����
	uint8_t  packInSOA;// �����SOA��
	uint8_t  watchDogTime;// ���Ź�ʱ��
	driverLTC6811CellsTypedef cellVoltagesIndividual[NoOfCellsPossibleOnChip];// ��ص�ѹ
	modPowerElectronicsPackOperationalCellStatesTypedef packOperationalCellState;// ��ز���״̬0000
	
	// Slave BMS
	uint8_t  hiAmpShieldPresent;// �ߵ�����������
	float    hiCurrentLoadVoltage;// �ߵ������ص�ѹ
	float    hiCurrentLoadCurrent;// �ߵ������ص���
	float		 hiCurrentLoadPower;// �ߵ������ع���
	float    auxVoltage;// ������ѹ
	float    auxCurrent;// ��������
	float		 auxPower;// ��������
	uint8_t  aux0EnableDesired;// ����0ʹ������
	uint8_t  aux0Enabled;// ����0ʹ��
	uint8_t  aux0LoadIncorrect;// ����0���ش���
	uint8_t  aux1EnableDesired;// ����1ʹ������
	uint8_t  aux1Enabled;// ����1ʹ��
	uint8_t  aux1LoadIncorrect;// ����1���ش���
	uint8_t  auxDCDCEnabled;// ����DCDCʹ��
	uint8_t  auxDCDCOutputOK;// ����DCDC�������
	float    humidity;// ʪ��
	uint8_t  hiLoadEnabled;// �ߵ�������ʹ��
	uint8_t  hiLoadPreChargeEnabled;// �ߵ�������Ԥ���ʹ��
	uint8_t  hiLoadPreChargeError;// �ߵ�������Ԥ������
	uint8_t	 IOIN1;// IO����1
	uint8_t  IOOUT0;// IO���0
	uint8_t  FANSpeedDutyDesired;// ����ת��ռ�ձ�����
	driverSWEMC2305FanStatusTypeDef FANStatus;// ����״̬
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



