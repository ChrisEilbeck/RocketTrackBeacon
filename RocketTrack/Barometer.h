
#pragma once

extern int baro_enable;
extern int baro_rate;
extern int baro_gps_sync;
extern bool baro_trigger;

int SetupBarometer(void);
void PollBarometer(void);

int BarometerCommandHandler(uint8_t *cmd,uint16_t cmdptr);

typedef enum
{
	NO_BARO=0,
	BME280,
	BMP280
} baro_type;