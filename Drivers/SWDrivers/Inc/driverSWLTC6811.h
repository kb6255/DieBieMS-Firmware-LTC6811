#ifndef DRIVERSW_LTC6811_H_
#define DRIVERSW_LTC6811_H_

#include "driverHWSPI1.h"
#include "stdlib.h"
#include "math.h"
#include "stdint.h"
#include "stdbool.h"

#define LTC6811_REG_GROUP_SIZE 12

typedef struct {
	float cellVoltage;
	uint8_t cellNumber;
} driverLTC6811CellsTypedef;

typedef struct {
	uint16_t DisChargeEnableMask;
	uint8_t  noOfCells;
	float    CellUnderVoltageLimit;
	float    CellOverVoltageLimit;
	uint8_t  ADCOPT;
	uint8_t  REFON;
	uint8_t  GPIO[5];
	uint8_t  CDCMode;
} driverLTC6811ConfigStructTypedef;

void    driverSWLTC6811Init(driverLTC6811ConfigStructTypedef configStruct, uint8_t totalNumberOfLTCs);
void    driverSWLTC6811ReInit(void);
void    driverSWLTC6811StartCellVoltageConversion(void);
void    driverSWLTC6811StartTemperatureVoltageConversion(void);
void    driverSWLTC6811ResetCellVoltageRegisters(void);
bool    driverSWLTC6811ReadCellVoltages(driverLTC6811CellsTypedef cellVoltages[12]);
bool    driverSWLTC6811ReadTempVoltages(uint16_t tempVoltages[3]);
void    driverSWLTC6811WriteConfigRegisters(uint8_t total_ic, uint8_t config[][6]);
void    driverSWLTC6811WriteConfig(driverLTC6811ConfigStructTypedef configStruct);
bool    driverSWLTC6811ReadConfigRegisters(uint8_t total_ic, uint8_t r_config[][8]);
bool    driverSWLTC6811ReadConfig(driverLTC6811ConfigStructTypedef *configStruct);
bool    driverSWLTC6811ReadFlagRegisters(uint8_t total_ic, uint8_t flagRegisters[][8]);
bool    driverSWLTC6811ReadVoltageFlags(uint16_t *underVoltageFlags, uint16_t *overVoltageFlags);
void    driverSWLTC6811EnableBalanceResistors(uint16_t balanceEnableMask);

uint16_t pec15_calc(uint8_t len, uint8_t *data);

void     driverSWLTC6811WakeUp(uint8_t total_ic);
void     driverSWLTC6811SendCmd(uint8_t cmd0, uint8_t cmd1);
void     driverSWLTC6811Write(uint8_t *writeBytes, uint8_t writeLength);
void     driverSWLTC6811WriteRead(uint8_t *writeBytes, uint8_t writeLength, uint8_t *readBytes, uint8_t readLength);

float    driverSWLTC6811ConvertTemperatureExt(uint16_t inputValue, uint32_t ntcNominal, uint32_t ntcSeriesResistance, uint16_t ntcBetaFactor, float ntcNominalTemp);
float    driverSWLTC6811ConvertTemperatureInt(uint16_t inputValue);

#endif /* DRIVERSW_LTC6811_H_ */
