
#pragma once

#define EEPROM_SIZE			256

#define NVMEMORY_ARRAY		0x0000

int SetupNvMemory(void);

void StoreCalibrationData(void);
int RetrieveCalibrationData(void);

