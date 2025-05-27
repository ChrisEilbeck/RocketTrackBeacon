
#include "Logging.h"

// an array of fix structures to hold the last packet from each beacon
// we're tracking
fix beacons[MAX_BEACONS];

// use the same structure to hold the receiver location
fix rxfix={0,0,0,0,0,0,0,0,0,0.0,0.0,0.0,0.0,0,0,0,0};

// this is a scratch structure which'll be populated as a packet is decoded
// then copied into the right beacon slot once we have the id of the beacon
// it came from
//
// insert the location of Bredon Hill so that's the default reported
// location until we receive packets from any beacon
fix lastfix={7,3,6,13,-63,0,0,0,0,52.059956,-2.064869,500.0,1.23,3.99,0.0,1234,0};

void PackPacket(uint8_t *TxPacket,uint16_t *TxPacketLength)
{
#if 0
	Serial.print("PackPacket()\r\n");
#endif
	
	static uint16_t packetcounter=0;
	uint8_t packet[16];

#if 0	
	Serial.printf("TX latitude = %.6f\r\n",lastfix.latitude);
	Serial.printf("TX longitude = %.6f\r\n",lastfix.longitude);
#endif
#if 0
	float latitude=lastfix.latitude/1e7;
	float longitude=lastfix.longitude/1e7;
	int32_t packedlat=*(int32_t *)&latitude;
	int32_t packedlon=*(int32_t *)&longitude;
#else
	int32_t packedlat=(int32_t)(131072.0*lastfix.latitude/1e7);
	int32_t packedlon=(int32_t)(131072.0*lastfix.longitude/1e7);
#endif
#if 0	
	Serial.printf("packedlat = %d\r\n",packedlat);
	Serial.printf("packedlon = %d\r\n",packedlon);
#endif
	
	double hght=(double)lastfix.height/1e3;
	int16_t packed_height=(int16_t)hght;
	
	int cnt;
	uint8_t numsats=0;
	
	for(cnt=0;cnt<MAX_CHANNELS;cnt++)
		if(cno[cnt]>0)
			numsats++;
	
	// just an id value
	packet[0]=lora_id;
	
	packet[1]=lastfix.numsats;
	packet[1]|=(lastfix.gpsfix&0x03)<<6;
	
	packet[2]=(packedlon&0x000000ff)>>0;	packet[3]=(packedlon&0x0000ff00)>>8;	packet[4]=(packedlon&0x00ff0000)>>16;	packet[5]=(packedlon&0xff000000)>>24;
	packet[6]=(packedlat&0x000000ff)>>0;	packet[7]=(packedlat&0x0000ff00)>>8;	packet[8]=(packedlat&0x00ff0000)>>16;	packet[9]=(packedlat&0xff000000)>>24;
	
	packet[10]=(packed_height&0x00ff)>>0;
	packet[11]=(packed_height&0xff00)>>8;
	
	// horizontal accuracy estimate from NAV-POSLLH message in units of 0.5m
	packet[12]=lastfix.accuracy;
	
	// battery voltage divided by 20 so 4250 would read as a 212 count, already scaled in PMIC.ino
	packet[13]=lastfix.voltage;
	
	packet[14]=(packetcounter&0x00ff)>>0;
	packet[15]=(packetcounter&0xff00)>>8;
	
	memcpy(TxPacket,packet,16);
	*TxPacketLength=16;
	
	packetcounter++;
}

void UnpackPacket(uint8_t *RxPacket,uint16_t RxPacketLength,int8_t rssi,int8_t snr,uint8_t rxmode)
{
#if DEBUG>2
	Serial.print("UnpackPacket()\r\n");
#endif
		
	lastfix.id=RxPacket[0];
	
	lastfix.numsats=RxPacket[1]&0x03f;
	lastfix.gpsfix=(RxPacket[1]&0xc0)>>6;
	
	int32_t longitude=RxPacket[2]+(RxPacket[3]<<8)+(RxPacket[4]<<16)+(RxPacket[5]<<24);
	lastfix.longitude=(float)longitude/131072.0;
	
	int32_t latitude=RxPacket[6]+(RxPacket[7]<<8)+(RxPacket[8]<<16)+(RxPacket[9]<<24);
	lastfix.latitude=(float)latitude/131072.0;
	
	int height=RxPacket[10]+(RxPacket[11]<<8);
	lastfix.height=(float)height;
	
	lastfix.accuracy=(float)(RxPacket[12]<1);
	
	lastfix.voltage=(float)(RxPacket[13]*20);
	
	lastfix.counter=RxPacket[14]+(RxPacket[15]<<8);
	
	lastfix.spare1=0x00;	lastfix.spare2=0x00;	lastfix.spare3=0x00;
	
	lastfix.rssi=rssi;
	lastfix.snr=snr;
	lastfix.rxmode=rxmode;
	
	// used for packet age calculate in web gui
	lastfix.millis=millis();
	
	StorePacketInBeaconBuffer(lastfix.id);

#if DEBUG>2
	DumpDecodedPacket(lastfix);
#endif
}

void DumpDecodedPacket(fix newfix)
{
	if(		(newfix.spare1!=0xff)
		&&	(newfix.spare2!=0xff)
		&&	(newfix.spare3!=0xff)	)
	{
		Serial.printf("\tBeacon ID:\t%d\r\n",newfix.id);
		Serial.printf("\tBeacon Latitude:\t%.6f\r\n",newfix.latitude);
		Serial.printf("\tBeacon Longitude:\t%.6f\r\n",newfix.longitude);
		Serial.printf("\tBeacon Height:\t%.6f\r\n",newfix.height);
		Serial.printf("\tBeacon Voltage:\t%.1f mV\r\n",newfix.voltage);
		Serial.printf("\tBeacon Accuracy:\t%.3f\r\n",newfix.accuracy);
		Serial.printf("\tBeacon Count Value:\t%d\r\n",newfix.counter);
	}
	else
	{
		Serial.println("Unused beacon buffer");
	}
}

void DumpHexPacket(uint8_t *packet,uint16_t packetlength)
{
	int cnt;
	for(cnt=0;cnt<packetlength;cnt++)
		Serial.printf("%02x",packet[cnt]);
	
	Serial.print("\r\n");
}

void StorePacketInBeaconBuffer(int id)
{
	// find if we have a beacon fix buffer already allocated for the beacon
	// we've just recieved from
	
	bool found=false;
	
	int cnt;
	for(cnt=0;cnt<MAX_BEACONS;cnt++)
	{
		if(		(beacons[cnt].spare1!=0xff)
			&&	(beacons[cnt].spare2!=0xff)
			&&	(beacons[cnt].spare3!=0xff)
			&&	(beacons[cnt].id==id)		)
		{
			// this is an occupied slot and the id matches the one we've
			// just received from so store the packet into that slotkd
			
			memcpy((void *)&beacons[cnt],(void *)&lastfix,sizeof(fix));
			Serial.printf("Packet stored to slot %d\r\n",cnt);
			found=true;
		}
	}
	
	// if we didn't find a packet already received from the given beacon,
	// just find the first empty slot and allocate that to this beacon
	
	if(!found)
	{
		Serial.printf("Beacon %d not already found in beacons buffers\r\n",id);
		
		for(cnt=0;cnt<MAX_BEACONS;cnt++)
		{
			if(		(beacons[cnt].spare1==0xff)
				&&	(beacons[cnt].spare2==0xff)
				&&	(beacons[cnt].spare3==0xff)		)
			{
				// this is an unoccupied slot so start using it
				memcpy((void *)&beacons[cnt],(void *)&lastfix,sizeof(fix));
				Serial.printf("Packet stored to slot %d\r\n",cnt);
				found=true;
				break;
			}
		}
	}
	
	// if we didn't find and empty slot, re-use the one with the oldest
	// received packet in it
	
	if(!found)
	{
		Serial.println("Looking for a slot to re-use");
		
		int oldest_fix=-1;
		int oldest_time=1<<31;
		
		for(cnt=0;cnt<MAX_BEACONS;cnt++)
		{
			if(beacons[cnt].millis<oldest_time)
			{
				oldest_time=beacons[cnt].millis;
				oldest_fix=cnt;
				Serial.printf("Oldest fix is in slot %d\r\n",cnt);
			}
		}
		
		cnt=oldest_fix;
		memcpy((void *)&beacons[cnt],(void *)&lastfix,sizeof(fix));
		Serial.printf("Packet stored to slot %d\r\n",cnt);
	}
}

