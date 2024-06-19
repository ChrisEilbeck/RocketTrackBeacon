
#pragma once

extern bool baro_enable;
extern char baro_type[];
extern int baro_rate;
extern bool baro_trigger;

int SetupBarometer(void);
void PollBarometer(void);

int BarometerCommandHandler(uint8_t *cmd,uint16_t cmdptr);

#if 0
	float ReadAltitude(void);
	float ReadPressure(void);
	float ReadTemperature(void);
	float ReadHumidity(void);
#endif

enum
{
	BAROMETER_NONE=0,
	BAROMETER_BME180,
	BAROMETER_BME280,
	BAROMETER_BMP085
};