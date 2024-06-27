
#pragma once

#define MAX_TX_PACKET_LENGTH 64

#if 0
	extern uint8_t beaconid;
	extern float beaconlat;
	extern float beaconlon;
	extern float beaconheight;
	extern float beaconvoltage;
	extern float beaconhacc;
	extern float beaconvacc;

	extern uint8_t beaconnumSats;

	extern uint8_t gpsFix;

	//extern int32_t beaconlat;
	//extern int32_t beaconlon;
	//extern int32_t beaconheight;
	extern int32_t beaconhMSL;
	extern int32_t max_beaconhMSL;
	//extern uint32_t beaconhacc;
	//extern uint32_t beaconvacc;

	extern uint8_t beaconhaccvalue;

	extern uint16_t beaconyear;
	extern uint8_t beaconmonth;
	extern uint8_t beaconday;
	extern uint8_t beaconhour;
	extern uint8_t beaconmin;
	extern uint8_t beaconsec;
#endif

void PackPacket(uint8_t *TxPacket,uint16_t *TxPacketLength);
void UnpackPacket(uint8_t *RxPacket,uint16_t RxPacketLength);
