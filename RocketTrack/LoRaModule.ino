
#include "HardwareAbstraction.h"

#if USE_RADIOHEAD
	#include <RadioHead.h>
	#include <RH_RF95.h>
	#include <RHSoftwareSPI.h>
#endif

#if USE_ARDUINO_LORA
	#include <LoRa.h>
#endif

#define LOW_POWER_TRANSMIT 1

#include <string.h>

#include "Packetisation.h"

bool LoRaTransmitSemaphore=false;
bool LoRaTXDoneSemaphore=false;
int LoRaBurstDuration=0;
uint32_t TXStartTimeMillis;

int tx_active=0;

// LORA settings
#define LORA_FREQ			433920000
#define LORA_OFFSET			4.185

// HARDWARE DEFINITION

#if USE_RADIOHEAD
	RHSoftwareSPI software_spi;
	RH_RF95 LoRa(LORA_NSS,LORA_DIO0,software_spi);
#endif

double lora_freq=LORA_FREQ;
double lora_offset=LORA_OFFSET;
char lora_mode[16]="High Rate";

int lora_crc=0;

int hr_bw=125000;
int hr_sf=7;
int hr_cr=8;
int hr_period=1000;

int lr_bw=31250;
int lr_sf=12;
int lr_cr=8;
int lr_period=10000;

bool lora_constant_transmit=false;

uint8_t TxPacket[MAX_TX_PACKET_LENGTH];
uint16_t TxPacketLength;

uint16_t TxPacketCounter=0;

uint32_t LastLoRaTX=0;

int SetupLoRa(void)
{
#if USE_ARDUINO_LORA
	Serial.println("Using Arduino LoRa");

    LoRa.setPins(LORA_NSS,LORA_RESET,LORA_DIO0);
	LoRa.onTxDone(onTxDone);
	
	if(!LoRa.begin(lora_freq))	{	Serial.println("Starting LoRa module failed!");	return(1);	}
	else						{	Serial.println("Started LoRa module ok ...");				}
#endif
#if USE_RADIOHEAD
	Serial.println("Using RadioHead");

	pinMode(LORA_RESET,OUTPUT);
	digitalWrite(LORA_RESET,LOW);
	delay(20);
	digitalWrite(LORA_RESET,HIGH);
	delay(20);

	pinMode(LORA_NSS,INPUT);
	software_spi.setPins(LORA_MISO,LORA_MOSI,LORA_SCK);
		
	if(!LoRa.init())			{	Serial.println("LoRa Radio: init failed.");		return(1);	}
	else
	{
		Serial.println("LoRa Radio: init OK!");						
		SetLoRaMode(lora_mode);
		
	#if USE_FREERTOS	
		xTaskCreatePinnedToCore(LoRaTask,"LoRa Task",2048,NULL,2,NULL,0);
//		xTaskCreate(LoRaTask,"LoRa Task",2048,NULL,2,NULL);
		Serial.println("Created LoRa Task");
	#endif
	}
#endif

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
#if USE_ARDUINO_LORA
					LoRa.dumpRegisters(Serial);
#endif
#if USE_RADIOHEAD
					LoRa.printRegisters();
#endif
					
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
					DumpHexPacket(TxPacket,TxPacketLength,true);
					EncryptPacket(TxPacket);
					DumpHexPacket(TxPacket,TxPacketLength,true);
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
					Serial.printf("LoRa offset = %.1f KHz\r\n",lora_offset);
					break;
		
		case '-':	lora_offset-=1.0;
					Serial.printf("LoRa offset = %.1f KHz\r\n",lora_offset);
					break;
		
		case 'v':	Serial.print("LoRa version: ");
					Serial.println(LoRa.getDeviceVersion());
					
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
	if(lora_freq>1e6)	lora_freq/=1e6;
	
	Serial.print("Setting frequency to ");
	Serial.print(lora_freq+lora_offset/1e3,3);
	Serial.println(" MHz");
	LoRa.setFrequency(lora_freq+lora_offset/1e3);

#if LOW_POWER_TRANSMIT
	LoRa.setTxPower(0);
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

#if 0	
	LoRa.setSpreadingFactor(7);
	LoRa.setSignalBandwidth(125000);
	LoRa.setCodingRate4(8);
#endif
#if USE_ARDUINO_LORA
	if(lora_crc)	LoRa.enableCrc();
	else			LoRa.disableCrc();
#endif
#if USE_RADIOHEAD
	LoRa.setPayloadCRC(lora_crc);
#endif
}

void PollLoRa(void)
{
	// scale to Hz if our data is in MHz
	if(lora_freq>1e6)	lora_freq/=1e6;
	
	if(LoRaTransmitSemaphore)
	{
		Serial.println("Starting tx ...");
//		TXStartTimeMillis=millis();

//		Serial.println("Setting tx_active");
//		tx_active=1;
//		SetTXIndicator(tx_active);
		
#if 0
		Serial.println("Setting LoRa parameters");
		SetLoRaMode(lora_mode);		
#endif		
#if 0
		Serial.println("Setting the transmit frequency");
		LoRa.setFrequency(lora_freq+lora_offset/1e3);
#endif
#if 0
#if USE_ARDUINO_LORA
		LoRa.beginPacket(false);
		LoRa.write(TxPacket,TxPacketLength);
		LoRa.endPacket(true);
#endif
#if USE_RADIOHEAD
		Serial.print("Sending ");
		DumpHexPacket(TxPacket,TxPacketLength,false);
		Serial.print(", ");
		Serial.print(TxPacketLength);
		Serial.println(" bytes");

		LoRa.send(TxPacket,TxPacketLength);
#endif
#endif
//		Serial.println("Setting the transmit semaphore off");
//		LoRaTransmitSemaphore=0;
	}

#if 0
	if(LoRaTXDoneSemaphore)
	{
		Serial.print("\t\tlora tx done in ");
		Serial.print(LoRaBurstDuration);
		Serial.print(" ms\r\n");
		LoRaTXDoneSemaphore=false;

		tx_active=0;
		SetTXIndicator(tx_active);
	}
#endif
}

void LoRaTask(void *pvParameters)
{
	while(1)
	{
		// scale to Hz if our data is in MHz
		if(lora_freq>1e6)	lora_freq/=1e6;
		
		if(LoRaTransmitSemaphore)
		{
			Serial.println("Starting tx ...");

//			tx_active=1;
//			SetTXIndicator(tx_active);

//			LoRa.setModemConfig(RH_RF95::Bw31_25Cr48Sf512);

#if 0			
			Serial.println("Setting LoRa parameters");
			SetLoRaMode(lora_mode);		
#endif
			
			Serial.println("Setting the transmit frequency");
			LoRa.setFrequency(lora_freq+lora_offset/1e3);

			TXStartTimeMillis=millis();
#if USE_ARDUINO_LORA
			LoRa.beginPacket(false);
			LoRa.write(TxPacket,TxPacketLength);
			LoRa.endPacket(true);
#endif
#if USE_RADIOHEAD
			Serial.print("Sending ");
			DumpHexPacket(TxPacket,TxPacketLength,false);
			Serial.print(", ");
			Serial.print(TxPacketLength);
			Serial.println(" bytes");

			LoRa.send(TxPacket,TxPacketLength);
#endif
			
			Serial.println("Waiting for packet sent ...");
			while(!LoRa.waitPacketSent(1))
				delay(1);
			
			Serial.print("Tx done in ");
			Serial.print(millis()-TXStartTimeMillis);
			Serial.println(" ms");
			
			Serial.println("Setting the transmit semaphore off");
			LoRaTransmitSemaphore=0;

			LoRaTXDoneSemaphore=false;
			tx_active=0;
		}

#if 0
		if(LoRaTXDoneSemaphore)
		{
			Serial.print("\t\tlora tx done in ");
			Serial.print(LoRaBurstDuration);
			Serial.print(" ms\r\n");
			LoRaTXDoneSemaphore=false;

			tx_active=0;
			SetTXIndicator(tx_active);
		}
#endif
				
		delay(1);
	}
}

