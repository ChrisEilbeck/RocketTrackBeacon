
#pragma once

extern bool mag_enable;
extern char mag_type[];
extern int mag_rate;

extern bool mag_trigger;

int SetupMagnetometer(void);
void PollMagnetometer(void);

void ReadMagnetometer(float *mag_x,float *mag_y,float *mag_z);

int MagnetometerCommandHandler(uint8_t *cmd,uint16_t cmdptr);

enum
{
	MAGNETOMETER_NONE=0,
	MAGNETOMETER_LSM303DLHC
};

