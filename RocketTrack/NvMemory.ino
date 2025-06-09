
#include <EEPROM.h>

#include "NvMemory.h"

int SetupNvMemory(void)
{
	EEPROM.begin(EEPROM_SIZE);
	return(0);
}

void PollNvMemory(void)
{
	// not used but retained as a common module structure
}

int NvMemoryCommandHandler(uint8_t *cmd,uint16_t cmdptr)
{
	// ignore a single key stroke
	if(cmdptr<=2)	return(0);

#if (DEBUG>0)
	Serial.println((char *)cmd);
#endif
	
	int retval=1;
	uint8_t cnt;
	
	switch(cmd[1]|0x20)
	{
		case 's':	// store to non-volatile memory
					StoreSettings();
					Serial.println("Settings stored");		
		
					break;
		
		case 'r':	// recall from non-volatile memory
		
					if(RetrieveSettings())
						Serial.println("Error recalling settings!");
					else
						Serial.println("Settings read ok");
		
					break;
	
		case 'x':	// hexdump of the eeprom for debugging
					{
						int cnt;
						for(cnt=0;cnt<EEPROM_SIZE;cnt++)
						{
							if((cnt%16)==0)
								Serial.print("\r\n");
							
							Serial.printf("%02x ",EEPROM.read(cnt));
						}
						
						Serial.print("\r\n");
					}
					
					break;
					
		case '?':	Serial.print("NvMemory Test Harness\r\n================\r\n\n");
					Serial.print("x\t-\tDump hex display of eeprom contents\r\n");
					Serial.print("?\t-\tShow this menu\r\n");
					break;
		
		default:	retval=0;
					break;
	}
	
	return(retval);
}

void StoreSettings(void)
{	
	int64_t addr=NVMEMORY_ARRAY;
	
	EEPROM.writeFloat(addr,AccelOffset.x);		addr+=sizeof(float);
	EEPROM.writeFloat(addr,AccelOffset.y);		addr+=sizeof(float);
	EEPROM.writeFloat(addr,AccelOffset.x);		addr+=sizeof(float);
	
	EEPROM.writeFloat(addr,AccelScale.x);		addr+=sizeof(float);
	EEPROM.writeFloat(addr,AccelScale.y);		addr+=sizeof(float);
	EEPROM.writeFloat(addr,AccelScale.z);		addr+=sizeof(float);
	
	EEPROM.writeFloat(addr,GyroOffset.x);		addr+=sizeof(float);
	EEPROM.writeFloat(addr,GyroOffset.y);		addr+=sizeof(float);
	EEPROM.writeFloat(addr,GyroOffset.z);		addr+=sizeof(float);
	
	EEPROM.writeFloat(addr,Mag_A11);			addr+=sizeof(float);
	EEPROM.writeFloat(addr,Mag_A12);			addr+=sizeof(float);
	EEPROM.writeFloat(addr,Mag_A13);			addr+=sizeof(float);
	EEPROM.writeFloat(addr,Mag_A21);			addr+=sizeof(float);
	EEPROM.writeFloat(addr,Mag_A22);			addr+=sizeof(float);
	EEPROM.writeFloat(addr,Mag_A23);			addr+=sizeof(float);
	EEPROM.writeFloat(addr,Mag_A31);			addr+=sizeof(float);
	EEPROM.writeFloat(addr,Mag_A32);			addr+=sizeof(float);
	EEPROM.writeFloat(addr,Mag_A33);			addr+=sizeof(float);

	EEPROM.writeFloat(addr,Mag_B1);				addr+=sizeof(float);
	EEPROM.writeFloat(addr,Mag_B2);				addr+=sizeof(float);
	EEPROM.writeFloat(addr,Mag_B3);				addr+=sizeof(float);

	EEPROM.writeFloat(addr,lora_freq);			addr+=sizeof(float);
	
	EEPROM.writeInt(addr,hr_bw);				addr+=sizeof(int);
	EEPROM.writeInt(addr,hr_sf);				addr+=sizeof(int);
	EEPROM.writeInt(addr,hr_cr);				addr+=sizeof(int);

	EEPROM.writeInt(addr,lr_bw);				addr+=sizeof(int);
	EEPROM.writeInt(addr,lr_sf);				addr+=sizeof(int);
	EEPROM.writeInt(addr,lr_cr);				addr+=sizeof(int);
	
	EEPROM.writeInt(addr,lora_id);				addr+=sizeof(int);
	
	EEPROM.commit();
	
	Serial.print("Settings stored to NvMemory\r\n");
}

int RetrieveSettings(void)
{
	int64_t addr=NVMEMORY_ARRAY;
	
	float accoffx=EEPROM.readFloat(addr);		addr+=sizeof(float);
	float accoffy=EEPROM.readFloat(addr);		addr+=sizeof(float);
	float accoffz=EEPROM.readFloat(addr);		addr+=sizeof(float);
	
	float accscalex=EEPROM.readFloat(addr);		addr+=sizeof(float);
	float accscaley=EEPROM.readFloat(addr);		addr+=sizeof(float);
	float accscalez=EEPROM.readFloat(addr);		addr+=sizeof(float);
	
	float gyrox=EEPROM.readFloat(addr);			addr+=sizeof(float);
	float gyroy=EEPROM.readFloat(addr);			addr+=sizeof(float);
	float gyroz=EEPROM.readFloat(addr);			addr+=sizeof(float);

	float a11=EEPROM.readFloat(addr);			addr+=sizeof(float);
	float a12=EEPROM.readFloat(addr);			addr+=sizeof(float);
	float a13=EEPROM.readFloat(addr);			addr+=sizeof(float);
	float a21=EEPROM.readFloat(addr);			addr+=sizeof(float);
	float a22=EEPROM.readFloat(addr);			addr+=sizeof(float);
	float a23=EEPROM.readFloat(addr);			addr+=sizeof(float);
	float a31=EEPROM.readFloat(addr);			addr+=sizeof(float);
	float a32=EEPROM.readFloat(addr);			addr+=sizeof(float);
	float a33=EEPROM.readFloat(addr);			addr+=sizeof(float);

	float b1=EEPROM.readFloat(addr);			addr+=sizeof(float);
	float b2=EEPROM.readFloat(addr);			addr+=sizeof(float);
	float b3=EEPROM.readFloat(addr);			addr+=sizeof(float);

	float freq=EEPROM.readFloat(addr);			addr+=sizeof(float);

	int hrbw=EEPROM.readInt(addr);				addr+=sizeof(int);
	int hrsf=EEPROM.readInt(addr);				addr+=sizeof(int);
	int hrcr=EEPROM.readInt(addr);				addr+=sizeof(int);
	int lrbw=EEPROM.readInt(addr);				addr+=sizeof(int);
	int lrsf=EEPROM.readInt(addr);				addr+=sizeof(int);
	int lrcr=EEPROM.readInt(addr);				addr+=sizeof(int);

	int id=EEPROM.readInt(addr);				addr+=sizeof(int);

	if(		isnan(accoffx)||isnan(accoffy)||isnan(accoffz)
		||	isnan(accscalex)||isnan(accscaley)||isnan(accscalez)
		||	isnan(gyrox)||isnan(gyroy)||isnan(gyroz)
		||	isnan(a11)||isnan(a12)||isnan(a13)
		||	isnan(a21)||isnan(a22)||isnan(a23)
		||	isnan(a31)||isnan(a32)||isnan(a33)
		||	isnan(b1)||isnan(b2)||isnan(b3)					)
	{
		return(1);
	}
	
	AccelOffset.x=accoffx;		AccelOffset.y=accoffy;		AccelOffset.z=accoffz;
	AccelScale.x=accscalex;		AccelScale.y=accscaley;		AccelScale.z=accscalez;
	GyroOffset.x=gyrox;			GyroOffset.y=gyroy;			GyroOffset.z=gyroz;	
	
	Mag_A11=a11;				Mag_A12=a12;				Mag_A13=a13;
	Mag_A21=a21;				Mag_A22=a22;				Mag_A23=a23;
	Mag_A31=a31;				Mag_A32=a32;				Mag_A33=a33;
	
	Mag_B1=b1;					Mag_B2=b2;					Mag_B3=b3;
	
	lora_freq=freq;
	lora_id=id;
	hr_bw=hrbw;	hr_sf=hrsf;	hr_cr=hrcr;
	lr_bw=lrbw;	lr_sf=lrsf;	lr_cr=lrcr;

#if 0
	Serial.println(hr_bw);
	Serial.println(hr_sf);
	Serial.println(hr_cr);
	Serial.println(lr_bw);
	Serial.println(lr_sf);
	Serial.println(lr_cr);
#endif
	
	Serial.print("Settings retrieved from NvMemory\r\n");

	return(0);
}

