
#define LOW_POWER_TRANSMIT 0

#include <string.h>

#include "LoRa.h"
#include "Packetisation.h"

//typedef enum {lmIdle, lmListening, lmSending} tLoRaMode;

bool LoRaTransmitSemaphore=false;
bool LoRaTXDoneSemaphore=false;
int LoRaBurstDuration=0;
uint32_t TXStartTimeMillis;

int tx_active=0;

// LORA settings
#define LORA_FREQ			433920000
#define LORA_OFFSET			0         // Frequency to add in kHz to make Tx frequency accurate

// HARDWARE DEFINITION

double lora_freq=LORA_FREQ;
double lora_offset=LORA_OFFSET;
char lora_mode[32]="High Rate";

int lora_crc=0;

int hr_bw=1;
int hr_sf=2;
int hr_cr=3;
int hr_period=4;

int lr_bw=5;
int lr_sf=6;
int lr_cr=7;
int lr_period=8;

bool lora_constant_transmit=false;

uint8_t TxPacket[MAX_TX_PACKET_LENGTH];
uint16_t TxPacketLength;

uint16_t TxPacketCounter=0;

uint32_t LastLoRaTX=0;

int SetupLoRa(void)
{
    LoRa.setPins(LORA_NSS,LORA_RESET,LORA_DIO0);
	LoRa.onTxDone(onTxDone);
	
	if(lora_freq<1e6)	lora_freq*=1e6;
	
	Serial.print("Setting LoRa frequency to ");
	Serial.print(lora_freq/1e6,3);
	Serial.println(" MHz");
	
	if(!LoRa.begin(lora_freq))
	{
		Serial.println("Starting LoRa module failed!");
		return(1);
	}
	
	Serial.println("Started LoRa module ok ...");
	
	SetLoRaMode(lora_mode);
	
	return(0);
}

void onTxDone()
{
	LoRaTXDoneSemaphore=true;
	LoRaBurstDuration=millis()-TXStartTimeMillis;
}

int LORACommandHandler(uint8_t *cmd,uint16_t cmdptr)
{
	// ignore a single key stroke
	if(cmdptr<=2)	return(0);

#if (DEBUG>0)
	Serial.println((char *)cmd);
#endif
	
	int retval=1;
	
	switch(cmd[1]|0x20)
	{
		case 'd':	Serial.println("Dumping LoRa registers");
					LoRa.dumpRegisters(Serial);
					break;
		
		case 'x':	Serial.print("High Rate  SF     - ");	Serial.println(hr_sf);
					Serial.print("           BW     - ");	Serial.println(hr_bw);
					Serial.print("           CR     - ");	Serial.println(hr_cr);
					Serial.print("           Period - ");	Serial.println(hr_period);
					Serial.print("Long Range SF     - ");	Serial.println(lr_sf);
					Serial.print("           BW     - ");	Serial.println(lr_bw);
					Serial.print("           CR     - ");	Serial.println(lr_cr);
					Serial.print("           Period - ");	Serial.println(lr_period);
					Serial.print("Frequency         - ");	Serial.println(lora_freq);
					Serial.print("Offset            - ");	Serial.println(lora_offset);
					if(lora_crc)	Serial.println("CRC Enabled");
					else			Serial.println("CRC Disabled");
		
					break;
		
		case 't':	Serial.println("Transmitting LoRa packet");
					memcpy(TxPacket,"Hello, world ...",16);
					EncryptPacket(TxPacket);
					TxPacketLength=16;
					LoRaTransmitSemaphore=1;
					
					if(strcmp(lora_mode,"Long Range")==0)	led_control(0xf0f0f0f0,0);
					else									led_control(0xaa000000,0);
					
					break;
					
		case 'g':	Serial.println("Transmitting GPS LoRa packet");
					PackPacket(TxPacket,&TxPacketLength);
					EncryptPacket(TxPacket);
//					TxPacketLength=16;
					LoRaTransmitSemaphore=1;
					
					if(strcmp(lora_mode,"Long Range")==0)	led_control(0xf0f0f0f0,0);
					else									led_control(0xaa000000,0);
					
					break;
		
		case 'l':	Serial.println("Long range mode");
					strcpy(lora_mode,"Long Range");
					SetLoRaMode(lora_mode);
					break;
		
		case 'h':	Serial.println("High rate mode");
					strcpy(lora_mode,"High Rate");
					SetLoRaMode(lora_mode);
					break;
		
		case 'm':	Serial.print(lora_mode);
					Serial.print(" mode\r\n");
					break;
					
		case 'c':	lora_constant_transmit=!lora_constant_transmit;
					Serial.printf("Setting constant transmit mode to %d\r\n",lora_constant_transmit);
					break;
		
		case '+':	lora_offset+=1.0;
					Serial.printf("LoRa offset = %.1f\n",lora_offset);
					break;
		
		case '-':	lora_offset-=1.0;
					Serial.printf("LoRa offset = %.1f\n",lora_offset);
					break;
		
		case '?':	Serial.print("LoRa Test Harness\r\n================\r\n\n");
					Serial.print("t\t-\tTransmit a test packet\r\n");
					Serial.print("g\t-\tTransmit a GPS packet\r\n");
					Serial.print("h\t-\tSet high rate mode\r\n");
					Serial.print("l\t-\tSet long range mode\r\n");
					Serial.print("m\t-\tCheck operating mode\r\n");
					Serial.print("c\t-\tConstant Transmit on/off\r\n");
					Serial.print("+\t-\tIncrement LoRa offset\r\n");
					Serial.print("-\t-\tDecrement LoRa offset\r\n");
					Serial.print("?\t-\tShow this menu\r\n");
					break;
					
		default:	// ignore
					break;
	}
	
	return(retval);
}

int LongRangeCommandHandler(uint8_t *cmd,uint16_t cmdptr)
{
	// ignore a single key stroke
	if(cmdptr<=2)	return(0);

#if (DEBUG>0)
	Serial.println((char *)cmd);
#endif
	
	int retval=1;
	
	switch(cmd[1]|0x20)
	{
	
		default:	// ignore
					break;
	}
	
	return(retval);
}

int HighRateCommandHandler(uint8_t *cmd,uint16_t cmdptr)
{
	// ignore a single key stroke
	if(cmdptr<=2)	return(0);

#if (DEBUG>0)
	Serial.println((char *)cmd);
#endif
	
	int retval=1;
	
	switch(cmd[1]|0x20)
	{
	
		default:	// ignore
					break;
	}
	
	return(retval);
}

void SetLoRaMode(char *mode)
{
#if LOW_POWER_TRANSMIT
	LoRa.setTxPower(5);
#else
	LoRa.setTxPower(17);
#endif
	
	if(strcmp(mode,"Long Range")==0)
	{
		Serial.println("Setting LoRa to long range mode");
					
		LedPattern=0xf0f0f000;
		LedRepeatCount=0;
		LedBitCount=0;					
		
		LoRa.setSpreadingFactor(lr_sf);
		LoRa.setSignalBandwidth(lr_bw);
		LoRa.setCodingRate4(lr_cr);
	}
	else if(strcmp(mode,"High Rate")==0)
	{
		Serial.println("Setting LoRa to high rate mode");
					
		LedPattern=0xaaa00000;
		LedRepeatCount=0;
		LedBitCount=0;					
		
		LoRa.setSpreadingFactor(hr_sf);
		LoRa.setSignalBandwidth(hr_bw);
		LoRa.setCodingRate4(hr_cr);
	}
	else
	{	
		Serial.println("Duff LoRa mode selected!");
	}
	
	if(lora_crc)	LoRa.enableCrc();
	else			LoRa.disableCrc();
}

void PollLoRa(void)
{
	// scale to Hz if our data is in MHz
	if(lora_freq<1e6)	lora_freq*=1e6;
	
	if(LoRaTransmitSemaphore)
	{
		Serial.print("Starting tx ...");
		TXStartTimeMillis=millis();

		tx_active=1;
		SetTXIndicator(tx_active);
		
#if 0
		Serial.println("Setting LoRa parameters");
		LoRa.setTxPower(5);
		SetLoRaMode(lora_mode);		
#endif		

		LoRa.setFrequency(lora_freq);
		
		LoRa.beginPacket(false);
		LoRa.write(TxPacket,TxPacketLength);
		LoRa.endPacket(true);
		
		LoRaTransmitSemaphore=0;
	}

	if(LoRaTXDoneSemaphore)
	{
		Serial.print("\t\tlora tx done in ");
		Serial.print(LoRaBurstDuration);
		Serial.print(" ms\r\n");
		LoRaTXDoneSemaphore=false;

		tx_active=0;
		SetTXIndicator(tx_active);
	}
}

