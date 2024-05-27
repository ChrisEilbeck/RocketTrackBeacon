
#pragma once

extern int mag_enable;
extern char mag_type[];
extern int mag_rate;

int SetupMagnetometer(void);
void PollMagnetometer(void);

enum
{
	MAGNETOMETER_NONE=0,
	MAGNETOMETER_LSM303DLHC
};

