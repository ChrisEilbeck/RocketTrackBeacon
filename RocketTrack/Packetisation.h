
#pragma once

#include "Logging.h"

extern fix lastfix;
extern fix rxfix;

extern fix beacons[];

#define MAX_TX_PACKET_LENGTH 16

void PackPacket(uint8_t *TxPacket,uint16_t *TxPacketLength);
void UnpackPacket(uint8_t *RxPacket,uint16_t RxPacketLength,int8_t rssi,int8_t snr,uint8_t rxmode);
