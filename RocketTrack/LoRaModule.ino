
#define LOW_POWER_TRANSMIT 0

#include <string.h>

#if defined(ARDUINO_XIAO_ESP32S3)
	#define USE_RADIOHEAD 1
#endif
#if defined(ARDUINO_HELTEC_WIRELESS_TRACKER)
	#define USE_RADIOHEAD 1
#endif
#if defined(ARDUINO_HELTEC_WIRELESS_STICK_V3)
	#define USE_RADIOHEAD 1
#endif 
#if defined(ARDUINO_RASPBERRY_PI_PICO)
	#define USE_RADIOHEAD 1
#endif

#if USE_RADIOHEAD
	#warning "Using Radiohead library for sx126x chip"
	#include <RH_SX126x.h>
#else
	#include "LoRa.h"
#endif

// this is the custom packet format we're using for RocketTrack
#include "Packetisation.h"

bool lora_enable=true;

bool LoRaTransmitSemaphore=false;
bool LoRaTXDoneSemaphore=false;
int LoRaBurstDuration=0;
uint32_t TXStartTimeMillis;

int tx_active=0;

// LORA settings
#define LORA_FREQ			434150000
#define LORA_OFFSET			0         // Frequency to add in kHz to make Tx frequency accurate

// HARDWARE DEFINITION

float lora_freq=LORA_FREQ;

int lora_id=0;

double lora_offset=LORA_OFFSET;
char lora_mode[32]="High Rate";

int lora_crc=0;

// High Rate mode settings

int hr_bw=125000;
int hr_sf=7;
int hr_cr=8;
int hr_period=1000;		// not used in RocketTrackReceiver

// Long Range mode settings

int lr_bw=31250;
int lr_sf=12;
int lr_cr=8;
int lr_period=30000;	// not used in RocketTrackReceiver

#if USE_RADIOHEAD
	RH_SX126x::ModemConfig highrate={
		RH_SX126x::PacketTypeLoRa,
		RH_SX126x_LORA_SF_128,
		RH_SX126x_LORA_BW_125_0,
		RH_SX126x_LORA_CR_4_8,
		RH_SX126x_LORA_LOW_DATA_RATE_OPTIMIZE_OFF,
		0,
		0,
		0,
		0
	};
	
	RH_SX126x::ModemConfig longrange={
		RH_SX126x::PacketTypeLoRa,
		RH_SX126x_LORA_SF_4096,
		RH_SX126x_LORA_BW_31_25,
		RH_SX126x_LORA_CR_4_8,
		RH_SX126x_LORA_LOW_DATA_RATE_OPTIMIZE_OFF,
		0,
		0,
		0,
		0
	};
#endif

bool lora_constant_transmit=false;

uint8_t TxPacket[MAX_TX_PACKET_LENGTH];
uint16_t TxPacketLength;

uint16_t TxPacketCounter=0;

uint32_t LastLoRaTX=0;

#if USE_RADIOHEAD
	// NSS, DIO1, BUSY, NRESET
	RH_SX126x LoRaDriver(LORA_NSS,LORA_DIO0,LORA_BUSY,LORA_RESET);
#endif

int SetupLoRa(void)
{
	Serial.println("SetupLoRa() entry");

	// HACK
//	lora_freq=868.150;
	
	if(lora_freq<1e6)	lora_freq*=1e6;
	
	Serial.print("Setting LoRa frequency to ");
	Serial.print(lora_freq/1e6,3);
	Serial.println(" MHz");

	Serial.print("LoRa ID set to ");
	Serial.println(lora_id);
	
#if USE_RADIOHEAD
	if(!LoRaDriver.init())
	{
		Serial.println("Starting sx126x LoRa module failed!");
		return(1);	
	}

	// units are in MHz, not in Hz like other libraries
	LoRaDriver.setFrequency(lora_freq/1e6);
	
	// get rid of useless headers
	LoRaDriver.enableRawMode(true);
#else
    LoRa.setPins(LORA_NSS,LORA_RESET,LORA_DIO0);
	LoRa.onTxDone(onTxDone);
	
	if(!LoRa.begin(lora_freq))
	{
		Serial.println("Starting sx127x LoRa module failed!");
		return(1);
	}
#endif
		
	Serial.println("Started LoRa module ok ...");
	
//	SetLoRaMode(lora_mode);
	
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
		case '1':	lora_freq=LORA_CH1;
					Serial.println("Setting to LoRa Channel 1");
					retval=1;
					break;

		case '2':	lora_freq=LORA_CH2;
					Serial.println("Setting to LoRa Channel 2");
					retval=1;
					break;

		case '3':	lora_freq=LORA_CH3;
					Serial.println("Setting to LoRa Channel 3");
					retval=1;
					break;

		case '4':	lora_freq=LORA_CH4;
					Serial.println("Setting to LoRa Channel 4");
					retval=1;
					break;

		case '5':	lora_freq=LORA_CH5;
					Serial.println("Setting to LoRa Channel 5");
					retval=1;
					break;

		case '6':	lora_freq=LORA_CH6;
					Serial.println("Setting to LoRa Channel 6");
					retval=1;
					break;

		case 'c':	lora_constant_transmit=!lora_constant_transmit;
					Serial.printf("Setting constant transmit mode to %d\r\n",lora_constant_transmit);
					break;
		
#if USE_RADIOHEAD
#else
		case 'd':	Serial.println("Dumping LoRa registers");
					LoRa.dumpRegisters(Serial);
					break;
#endif
		
		case 'g':	Serial.println("Transmitting GPS LoRa packet");
					PackPacket(TxPacket,&TxPacketLength);
					EncryptPacket(TxPacket);
					LoRaTransmitSemaphore=1;
					
					if(strcmp(lora_mode,"Long Range")==0)	led_control(0xf0f0f0f0,0);
					else									led_control(0xaa000000,0);
					
					break;
		
		case 'h':	Serial.println("High rate mode");
					strcpy(lora_mode,"High Rate");
					SetLoRaMode(lora_mode);
					break;
		
		case 'i':	lora_id=atoi((const char *)&cmd[3]);
					Serial.print("Setting LoRa beacon id to ");
					Serial.println(lora_id);
					break;
		
		case 'l':	Serial.println("Long range mode");
					strcpy(lora_mode,"Long Range");
					SetLoRaMode(lora_mode);
					break;
		
		case 'm':	Serial.print(lora_mode);
					Serial.print(" mode\r\n");
					break;
					
		case 't':	Serial.println("Transmitting LoRa packet");
					memcpy(TxPacket,"Hello, world ...",16);
					EncryptPacket(TxPacket);
					TxPacketLength=16;
					LoRaTransmitSemaphore=1;
					
					if(strcmp(lora_mode,"Long Range")==0)	led_control(0xf0f0f0f0,0);
					else									led_control(0xaa000000,0);
					
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
		
		case '+':	lora_offset+=100.0;
					Serial.printf("LoRa offset = %.1f\n",lora_offset);
					break;
		
		case '-':	lora_offset-=100.0;
					Serial.printf("LoRa offset = %.1f\n",lora_offset);
					break;
		
		case '?':	Serial.print("LoRa Test Harness\r\n================\r\n\n");
					Serial.print("1..6\t-\tSet LoRa Channel\r\n");
					Serial.print("c\t-\tConstant Transmit on/off\r\n");
					Serial.print("d\t-\tDump LoRa registers\r\n");
					Serial.print("g\t-\tTransmit a GPS packet\r\n");
					Serial.print("h\t-\tSet high rate mode\r\n");
					Serial.print("l\t-\tSet long range mode\r\n");
					Serial.print("m\t-\tCheck operating mode\r\n");
					Serial.print("t\t-\tTransmit a test packet\r\n");
					Serial.print("x\t-\tDisplay LoRa parameters\r\n");
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
#if USE_RADIOHEAD
#else
	LoRa.setTxPower(17);
#endif
		
	if(strcmp(mode,"Long Range")==0)
	{
		Serial.println("Setting LoRa to long range mode");
					
		LedPattern=0xf0f0f000;
		LedRepeatCount=0;
		LedBitCount=0;					
		
#if USE_RADIOHEAD
		LoRaDriver.setFrequency(lora_freq/1e6,true);
		LoRaDriver.setModemRegisters(&longrange);
#else
		LoRa.setSpreadingFactor(lr_sf);
		LoRa.setSignalBandwidth(lr_bw);
		LoRa.setCodingRate4(lr_cr);
#endif
	}
	else if(strcmp(mode,"High Rate")==0)
	{
		Serial.println("Setting LoRa to high rate mode");
					
		LedPattern=0xaaa00000;
		LedRepeatCount=0;
		LedBitCount=0;					
		
#if USE_RADIOHEAD
		LoRaDriver.setFrequency(lora_freq/1e6,true);
		LoRaDriver.setModemRegisters(&highrate);
#else
		LoRa.setSpreadingFactor(hr_sf);
		LoRa.setSignalBandwidth(hr_bw);
		LoRa.setCodingRate4(hr_cr);
#endif
	}
	else
	{	
		Serial.println("Duff LoRa mode selected!");
	}
	
#if USE_RADIOHEAD
#else
	if(lora_crc)	LoRa.enableCrc();
	else			LoRa.disableCrc();
#endif
}

void PollLoRa(void)
{
	if(!lora_enable)
		return;

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

#if USE_RADIOHEAD
		Serial.printf("\tSetting LoRa frequency to %.3f MHz\r\n",lora_freq/1e6);
		LoRaDriver.setFrequency(lora_freq/1e6,false);
		
		Serial.println("Sending packet");
		LoRaDriver.send(TxPacket,TxPacketLength);
#else
		LoRa.setFrequency(lora_freq);
		
		LoRa.beginPacket(false);
		LoRa.write(TxPacket,TxPacketLength);
		LoRa.endPacket(true);
#endif
		
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

