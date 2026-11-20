#include "modStateOfCharge.h"
#include "modDelay.h"
#include <math.h>

modStateOfChargeStructTypeDef modStateOfChargeGeneralStateOfCharge;
modPowerElectricsPackStateTypedef *modStateOfChargePackStatehandle;
modConfigGeneralConfigStructTypedef *modStateOfChargeGeneralConfigHandle;
uint32_t modStateOfChargeLargeCoulombTick;
uint32_t modStateOfChargeStoreSoCTick;

bool modStateOfChargePowerDownSavedFlag = false;
bool modStateOfChargeInitialEstimateDone = false;

// Open circuit voltage lookup table for LiFePO4 (LFP) cells {rested cell voltage [V], SoC [%]}.
// Only used as a coarse estimate while the pack is at rest, coulomb counting does the fine tracking.
// WARNING: the LFP discharge plateau is very flat, so between roughly 15 and 90 percent
// small voltage differences map to large SoC changes. Treat the estimate accordingly.
static const float modStateOfChargeOcvTable[13][2] = {
	{2.90f,   0.0f},
	{3.00f,   3.0f},
	{3.08f,   6.0f},
	{3.13f,  10.0f},
	{3.16f,  15.0f},
	{3.19f,  25.0f},
	{3.22f,  40.0f},
	{3.25f,  55.0f},
	{3.28f,  70.0f},
	{3.31f,  82.0f},
	{3.34f,  90.0f},
	{3.37f,  96.0f},
	{3.40f, 100.0f},
};

// Minimum rested cell voltage above which the OCV estimate is considered meaningful.
static const float modStateOfChargeOcvMinValidVoltage = 3.00f;

static uint32_t modStateOfChargeRestTimerTick;
static uint32_t modStateOfChargeOcvCorrectionTick;

static float modStateOfChargeEstimateSoCFromVoltage(float cellVoltage) {
	const uint8_t tableSize = sizeof(modStateOfChargeOcvTable)/sizeof(modStateOfChargeOcvTable[0]);

	if(cellVoltage <= modStateOfChargeOcvTable[0][0])
		return 0.0f;

	if(cellVoltage >= modStateOfChargeOcvTable[tableSize-1][0])
		return 100.0f;

	for(uint8_t i=1 ; i<tableSize ; i++) {
		if(cellVoltage < modStateOfChargeOcvTable[i][0]) {
			float v0 = modStateOfChargeOcvTable[i-1][0];
			float v1 = modStateOfChargeOcvTable[i][0];
			float s0 = modStateOfChargeOcvTable[i-1][1];
			float s1 = modStateOfChargeOcvTable[i][1];
			return s0 + (cellVoltage - v0)*(s1 - s0)/(v1 - v0);
		}
	}

	return 0.0f;
};

modStateOfChargeStructTypeDef* modStateOfChargeInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer){
	modStateOfChargePackStatehandle = packState;
	modStateOfChargeGeneralConfigHandle = generalConfigPointer;
	driverSWStorageManagerStateOfChargeStructSize = (sizeof(modStateOfChargeStructTypeDef)/sizeof(uint16_t)); // Calculate the space needed for the config struct in EEPROM
	
	modStateOfChargeLargeCoulombTick = HAL_GetTick();
	modStateOfChargeStoreSoCTick = HAL_GetTick();
	modStateOfChargeRestTimerTick = HAL_GetTick();
	modStateOfChargeOcvCorrectionTick = HAL_GetTick();
	modStateOfChargeInitialEstimateDone = false;
	
	return &modStateOfChargeGeneralStateOfCharge;
};

void modStateOfChargeProcess(void){
	// --- Voltage based SoC estimation ------------------------------------------------
	// Stock firmware only tracks SoC by coulomb counting and sets 100 percent after a
	// completed charge cycle. With an empty EEPROM the SoC therefore starts at 0 percent
	// and stays wrong until the first full charge. Recover it from the measured minimum
	// cell voltage instead.

	float minCellVoltage = modStateOfChargePackStatehandle->cellVoltageLow;
	bool atRest = fabs(modStateOfChargePackStatehandle->packCurrent) < modStateOfChargeGeneralConfigHandle->notUsedCurrentThreshold;

	// 1) Initial estimate right after startup: stored SoC is (almost) empty but the lowest
	//    cell still shows a healthy voltage -> recover the real SoC from the OCV table.
	if(!modStateOfChargeInitialEstimateDone) {
		if(minCellVoltage > 0.5f) {
			if(minCellVoltage >= modStateOfChargeOcvMinValidVoltage && modStateOfChargeGeneralStateOfCharge.generalStateOfCharge < 5.0f) {
				float socEstimate = modStateOfChargeEstimateSoCFromVoltage(minCellVoltage);
				if(socEstimate > modStateOfChargeGeneralStateOfCharge.generalStateOfCharge) {
					modStateOfChargeGeneralStateOfCharge.remainingCapacityAh = socEstimate/100.0f*modStateOfChargeGeneralConfigHandle->batteryCapacity;
					modStateOfChargeGeneralStateOfCharge.generalStateOfCharge = socEstimate;
					modStateOfChargeStoreStateOfCharge();
				}
			}
			modStateOfChargeInitialEstimateDone = true;
		}
	}

	// 2) Periodic OCV correction while the pack is at rest: removes coulomb counting drift
	//    and wrong starting values. Rate limited so the displayed SoC never jumps.
	if(atRest) {
		if((uint32_t)(HAL_GetTick() - modStateOfChargeRestTimerTick) >= (10*60*1000)) {	// rested for at least 10 minutes
			if(modDelayTick1ms(&modStateOfChargeOcvCorrectionTick,60*1000)) {					// correct once per minute
				if(minCellVoltage >= modStateOfChargeOcvMinValidVoltage) {
					float socTarget = modStateOfChargeEstimateSoCFromVoltage(minCellVoltage);
					float capacityAh = modStateOfChargeGeneralConfigHandle->batteryCapacity;
					float targetAh  = socTarget/100.0f*capacityAh;
					float step = (targetAh - modStateOfChargeGeneralStateOfCharge.remainingCapacityAh)/10.0f;	// close 10 percent of the error per step

					if(step > 0.02f*capacityAh)		step = 0.02f*capacityAh;		// limit to max 2 percent per step
					if(step < -0.02f*capacityAh)	step = -0.02f*capacityAh;

					modStateOfChargeGeneralStateOfCharge.remainingCapacityAh += step;
				}
			}
		}
	}else{
		modStateOfChargeRestTimerTick = HAL_GetTick();
		modStateOfChargeOcvCorrectionTick = HAL_GetTick();
	}

	// Calculate accumulated energy
	uint32_t dt = HAL_GetTick() - modStateOfChargeLargeCoulombTick;
	modStateOfChargeStructTypeDef lastGeneralStateOfCharge;
	
	lastGeneralStateOfCharge = modStateOfChargeGeneralStateOfCharge;
	
	modStateOfChargeLargeCoulombTick = HAL_GetTick();
	modStateOfChargeGeneralStateOfCharge.remainingCapacityAh += dt*modStateOfChargePackStatehandle->packCurrent/(3600*1000);// (miliseconds * amps)/(3600*1000) accumulatedCharge in AmpHour.
	
	// Cap the max stored energy to the configured battery capacity.
	if(modStateOfChargeGeneralStateOfCharge.remainingCapacityAh > modStateOfChargeGeneralConfigHandle->batteryCapacity)
		modStateOfChargeGeneralStateOfCharge.remainingCapacityAh = modStateOfChargeGeneralConfigHandle->batteryCapacity;
	
	if(modStateOfChargeGeneralStateOfCharge.remainingCapacityAh < 0.0f)
		modStateOfChargeGeneralStateOfCharge.remainingCapacityAh = 0.0f;
	
	// Calculate state of charge
	modStateOfChargeGeneralStateOfCharge.generalStateOfCharge = modStateOfChargeGeneralStateOfCharge.remainingCapacityAh / modStateOfChargeGeneralConfigHandle->batteryCapacity * 100.0f;
	
	if(modStateOfChargeGeneralStateOfCharge.generalStateOfCharge >= 100.0f)
		modStateOfChargeGeneralStateOfCharge.generalStateOfCharge = 100.0f;
	
	modStateOfChargePackStatehandle->SoC = modStateOfChargeGeneralStateOfCharge.generalStateOfCharge;
	modStateOfChargePackStatehandle->SoCCapacityAh = modStateOfChargeGeneralStateOfCharge.remainingCapacityAh;
	
	// Store SoC every 'stateOfChargeStoreInterval'
	if(modDelayTick1ms(&modStateOfChargeStoreSoCTick,modStateOfChargeGeneralConfigHandle->stateOfChargeStoreInterval) && !modStateOfChargePowerDownSavedFlag && (lastGeneralStateOfCharge.remainingCapacityAh != modStateOfChargeGeneralStateOfCharge.remainingCapacityAh))
		modStateOfChargeStoreStateOfCharge();
};

bool modStateOfChargeStoreAndLoadDefaultStateOfCharge(void){
	bool returnVal = false;
	if(driverSWStorageManagerStateOfChargeEmpty){
		// TODO: SoC manager is empy -> Determin SoC from voltage when voltages are available.
		modStateOfChargeStructTypeDef defaultStateOfCharge;
		defaultStateOfCharge.generalStateOfCharge = 0.0f;
		defaultStateOfCharge.generalStateOfHealth = 0.0f;
		defaultStateOfCharge.remainingCapacityAh = 0.0f;
		defaultStateOfCharge.remainingCapacityWh = 0.0f;
		
		driverSWStorageManagerStateOfChargeEmpty = false;
		driverSWStorageManagerStoreStruct(&defaultStateOfCharge,STORAGE_STATEOFCHARGE);
		// TODO_EEPROM
	}
	
	modStateOfChargeStructTypeDef tempStateOfCharge;
	driverSWStorageManagerGetStruct(&tempStateOfCharge,STORAGE_STATEOFCHARGE);
	
	modStateOfChargeLoadStateOfCharge();
	return returnVal;
};

bool modStateOfChargeStoreStateOfCharge(void){
	return driverSWStorageManagerStoreStruct(&modStateOfChargeGeneralStateOfCharge,STORAGE_STATEOFCHARGE);
};

bool modStateOfChargeLoadStateOfCharge(void){
	return driverSWStorageManagerGetStruct(&modStateOfChargeGeneralStateOfCharge,STORAGE_STATEOFCHARGE);
};

bool modStateOfChargePowerDownSave(void) {
	if(!modStateOfChargePowerDownSavedFlag) {
		modStateOfChargePowerDownSavedFlag = true;
		modStateOfChargeStoreStateOfCharge();
		// TODO_EEPROM
		return true;
	}else
		return false;
};

void modStateOfChargeVoltageEvent(modStateOfChargeVoltageEventTypeDef eventType) {
	switch(eventType) {
		case EVENT_EMPTY:
			break;
		case EVENT_FULL:
			modStateOfChargeGeneralStateOfCharge.remainingCapacityAh = modStateOfChargeGeneralConfigHandle->batteryCapacity;
			break;
		default:
			break;
	}
};
