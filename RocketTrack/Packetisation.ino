
int lora_id=0;

// these are global variables to be populated from the last packet received
// and picked up by the web server

uint8_t beaconid=0;

#if 0
	// St Ives Lighthouse on Smeaton's Pier
	float ss.gps_latitude=50.213461;
	float ss.gps_longitude=-5.476708;
	float ss.gps_altitude=123.0;
#endif
#if 0
	// Bredon Hill
	float ss.gps_latitude=52.059956;
	float ss.gps_longitude=-2.064869;
	float ss.gps_altitude=500.0;
#endif

#if 0
	float bevoltage=4.200;
	float beaconhacc=1.23;
	uint8_t beaconnumsats=6;
	uint8_t beacongpsfix=3;
	uint16_t beaconcount=123;
	uint8_t beaconhaccvalue=1;
#endif

void PackPacket(uint8_t *TxPacket,uint16_t *TxPacketLength)
{
	Serial.print("PackPacket()\r\n");
	
	static uint16_t packetcounter=0;
	uint8_t packet[16];
	
	Serial.printf("beaconlat = %.6f\r\n",ss.gps_latitude);
	Serial.printf("beaconlon = %.6f\r\n",ss.gps_longitude);
	
	int32_t packedlat=(int32_t)(131072.0*ss.gps_latitude);
	int32_t packedlon=(int32_t)(131072.0*ss.gps_longitude);
	
	Serial.printf("packedlat = %d\r\n",packedlat);
	Serial.printf("packedlon = %d\r\n",packedlon);
	
	float hght=0.0f;
	if(baro_enable)		hght=ss.baro_altitude;
	else				hght=ss.gps_altitude;
	int16_t packed_height=(int16_t)hght;
	
	int cnt;
	uint8_t numsats=0;
	
//	for(cnt=0;cnt<MAX_CHANNELS;cnt++)
//		if(cno[cnt]>0)
//			numsats++;
	
	packet[0]=lora_id;	// just an id value
	
	packet[1]=ss.gps_numsats;
	packet[1]|=(ss.gps_fix&0x03)<<6;
	
	packet[2]=(packedlon&0x000000ff)>>0;	packet[3]=(packedlon&0x0000ff00)>>8;	packet[4]=(packedlon&0x00ff0000)>>16;	packet[5]=(packedlon&0xff000000)>>24;
	packet[6]=(packedlat&0x000000ff)>>0;	packet[7]=(packedlat&0x0000ff00)>>8;	packet[8]=(packedlat&0x00ff0000)>>16;	packet[9]=(packedlat&0xff000000)>>24;
	
	packet[10]=(packed_height&0x00ff)>>0;
	packet[11]=(packed_height&0xff00)>>8;
	
	// horizontal accuracy estimate from NAV-POSLLH message in units of 0.5m
	packet[12]=(int)(ss.gps_hdop*10.0f);
	
	// battery voltage divided by 20 so 4250 would read as a 212 count, already scaled in PMIC.ino
	packet[13]=ss.battery_voltage/20;
	
	packet[14]=(packetcounter&0x00ff)>>0;
	packet[15]=(packetcounter&0xff00)>>8;
	
	memcpy(TxPacket,packet,16);
	*TxPacketLength=16;
	
	packetcounter++;
}

void UnpackPacket(uint8_t *RxPacket,uint16_t RxPacketLength)
{
	Serial.print("UnpackPacket()\r\n");

#if 0	
	beaconid=RxPacket[0];
	
	ss.gps_numsats=RxPacket[1]&0x03f;
	ss.gps_fix=(RxPacket[1]&0xc0)>>6;
	
	int32_t longitude=RxPacket[2]+(RxPacket[3]<<8)+(RxPacket[4]<<16)+(RxPacket[5]<<24);
	beaconlon=(float)longitude*1e7/131072.0;
	
	int32_t latitude=RxPacket[6]+(RxPacket[7]<<8)+(RxPacket[8]<<16)+(RxPacket[9]<<24);
	ss.gps_latitude=(float)latitude/131072.0;
	
	int height=RxPacket[10]+(RxPacket[11]<<8);
	ss.received_altitude=(float)height;
	
	ss.gps_hdop=(float)(RxPacket[12]<1);
	
	ss.battery_voltage=(float)(RxPacket[13]*20);
	
	ss.packet_count=RxPacket[14]+(RxPacket[15]<<8);
	
#if 1
	Serial.printf("Beacon ID:\t%d\r\n",beaconid);
	Serial.printf("Beacon Lat:\t%.6f\r\n",beaconlat/1e7);
	Serial.printf("Beacon Lon:\t%.6f\r\n",beaconlon/1e7);
	Serial.printf("Beacon Hght:\t%.6f\r\n",beaconheight/1e3);
#endif
#endif
}

void DumpHexPacket(uint8_t *packet,uint16_t packetlength,bool newline)
{
	int cnt;
	for(cnt=0;cnt<packetlength;cnt++)
		Serial.printf("%02x",packet[cnt]);
	
	if(newline)
		Serial.print("\r\n");
}

