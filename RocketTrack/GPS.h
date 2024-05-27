
#pragma once

enum
{
	GPS_NMEA=0,
	GPS_UBLOX,
	GPS_MTK333x
	
} gps_type_numbers;

extern char gps_type[];
extern int initial_baud;
extern int final_baud;
extern int fix_rate;

extern uint8_t gpsFix;

#if 0
	extern uint8_t gps_numSats;

	extern int32_t gps_lat;
	extern int32_t gps_lon;
	extern int32_t gps_height;
	extern int32_t gps_hMSL;
	extern int32_t max_gps_hMSL;
	extern uint32_t gps_hAcc;
	extern uint32_t gps_vAcc;

	extern uint8_t gps_hAccValue;

	extern uint16_t gps_year;
	extern uint8_t gps_month;
	extern uint8_t gps_day;
	extern uint8_t gps_hour;
	extern uint8_t gps_min;
	extern uint8_t gps_sec;
#endif

int SetupGPS(void);

void PollGPS(void);

