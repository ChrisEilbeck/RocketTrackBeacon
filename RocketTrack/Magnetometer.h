
#pragma once

extern int mag_enable;
extern char mag_type[];
extern int mag_rate;

extern bool trigger_mag;

int SetupMagnetometer(void);
void PollMagnetometer(void);

void ReadMagnetometer(float *mag_x,float *mag_y,float *mag_z);

enum
{
	MAGNETOMETER_NONE=0,
	MAGNETOMETER_LSM303DLHC
};

