
#pragma once

extern char gps_type[];
extern int initial_baud;
extern int final_baud;
extern int fix_rate;

extern uint8_t gpsFix;

#if 0

	extern int32_t gps_lat;
	extern int32_t gps_lon;
	extern int32_t gps_height;
	extern int32_t gps_hMSL;
	extern int32_t max_gps_hMSL;
	extern uint32_t gps_hAcc;
	extern uint32_t gps_vAcc;

	extern uint8_t gps_hAccValue;
#endif

extern uint8_t beaconnumSats;
extern uint16_t beaconyear;
extern uint8_t beaconmonth;
extern uint8_t beaconday;
extern uint8_t beaconhour;
extern uint8_t beaconmin;
extern uint8_t beaconsec;
