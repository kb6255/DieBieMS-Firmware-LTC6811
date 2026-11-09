#include "driverSWStorageManager.h"

uint16_t driverHWEEPROMVirtAddVarTab[512];

bool driverSWStorageManagerConfigEmpty = false;
bool driverSWStorageManagerStateOfChargeEmpty = false;
uint16_t driverSWStorageManagerDataSize = 0;
uint16_t driverSWStorageManagerConfigStructSize = 0;
uint16_t driverSWStorageManagerStateOfChargeStructSize = 0;

void driverSWStorageManagerInit(void) {
	driverSWStorageManagerDataSize = driverSWStorageManagerConfigStructSize + driverSWStorageManagerStateOfChargeStructSize;
	
	for(uint16_t addresPointer=0 ; addresPointer<driverSWStorageManagerDataSize ; addresPointer++)
		driverHWEEPROMVirtAddVarTab[addresPointer] = addresPointer;
	
	HAL_FLASH_Unlock();
	
	if(driverHWEEPROMInit(driverSWStorageManagerDataSize) != HAL_OK)
		while(true);
	
	uint16_t readVariable;
	if(driverHWEEPROMReadVariable(driverHWEEPROMVirtAddVarTab[0], &readVariable) == 1) {
		driverSWStorageManagerConfigEmpty = true;
		driverSWStorageManagerStateOfChargeEmpty = true;
	}
};

bool driverSWStorageManagerEraseData(void) {
	return (driverHWEEPROMEraseFlash() == HAL_OK);
}

bool driverSWStorageManagerStoreStruct(void *configStruct, StorageLocationTypedef storageLocation) {
	uint8_t returnStatus = 0;
	uint16_t *dataPointer = (uint16_t*)configStruct;																																// Trick to convert struct to a 16 bit pointer.
	uint16_t dataOffset = driverSWStorageManagerGetOffsetFromLocation(storageLocation);															// Get addres offset
	uint16_t dataSize = driverSWStorageManagerGetStructSize(storageLocation);																				// Get data size
	
	for(uint16_t addresPointer=0 ; addresPointer<dataSize ; addresPointer++) {																			// Scan trough all adresses
		returnStatus |= driverHWEEPROMWriteVariable(driverHWEEPROMVirtAddVarTab[addresPointer+dataOffset],dataPointer[addresPointer]);// Store data in EEPROM
	}
	
	return (returnStatus == HAL_OK);
};

bool driverSWStorageManagerGetStruct(void *configStruct, StorageLocationTypedef storageLocation) {
	uint8_t returnStatus = 0;
	uint16_t *dataPointer = (uint16_t*)configStruct;																																// Trick to convert struct to a 16 bit pointer.
	uint16_t dataOffset = driverSWStorageManagerGetOffsetFromLocation(storageLocation);															// Get addres offset
	uint16_t dataSize = driverSWStorageManagerGetStructSize(storageLocation);																				// Get data size
	
	for(uint16_t addresPointer=0 ; addresPointer<dataSize ; addresPointer++)																				// Scan trough all adresses
		returnStatus |= driverHWEEPROMReadVariable(driverHWEEPROMVirtAddVarTab[addresPointer+dataOffset],&dataPointer[addresPointer]);// Get data from EEPROM
	
	return (returnStatus == HAL_OK);
};

uint16_t driverSWStorageManagerGetOffsetFromLocation(StorageLocationTypedef storageLocation) {
	int16_t locationPointer = -1;
	switch(storageLocation) {
		case STORAGE_CONFIG:
		  locationPointer = 0;																																												// StorageStruct starts at addres 0
			break;
		case STORAGE_STATEOFCHARGE:
			locationPointer = driverSWStorageManagerConfigStructSize;																										// StateOfCharge starts after configStruct
			break;
		default:
			break;
	};
	
	return locationPointer;
};

uint16_t driverSWStorageManagerGetStructSize(StorageLocationTypedef storageLocation) {
	int16_t sizeIndicator = -1;
	switch(storageLocation) {
		case STORAGE_CONFIG:
		  sizeIndicator = driverSWStorageManagerConfigStructSize;																											// Config struct size
			break;
		case STORAGE_STATEOFCHARGE:
			sizeIndicator = driverSWStorageManagerStateOfChargeStructSize;																							// StateOfCharge struct size
			break;
		default:
			break;
	};
	
	return sizeIndicator;
};
