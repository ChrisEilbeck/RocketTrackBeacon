
/*

to be done

-	logging format

-	wire up and integrate sd card

-	wire up sensors
		done
		
-	check low power

-	flight events (based on baro, not gps altitude, using gps location too)

-	turning off the gps after landing detection 
	
-	fsk backup mode if possible

-	using the OLED display for something useful other than just a logo

-	merge in the receiver code from RocketTrackReceiver and have one codebase that can be switched between roles using the config file on the sd card










*/ 

#include <axp20x.h>
#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
//#include "ESP32TimerInterrupt.h"

#include "HardwareAbstraction.h"

#include "Accelerometer.h"
#include "Barometer.h"
#include "Beeper.h"
#include "ConfigFile.h"
#include "Display.h"
#include "Gyro.h"
#include "Leds.h"
#include "Logging.h"
#include "Neopixels.h"
#include "SDCard.h"
#include "SpiffsSupport.h"
#include "Timers.h"
#include "Webserver.h"
#include "WiFiSupport.h"

extern char crypto_key_hex[65];
extern uint8_t crypto_key[32];

void setup()
{
	// Serial port(s)
	Serial.begin(115200);
	
#if 1
	#warning "wait for a serial connection, for development purposes only"
	while(!Serial);
#endif
	
	Serial.print("\n--------\tRocketTrack Flight Telemetry System\t--------\r\n\n");
	
	SPI.begin(SCK,MISO,MOSI);
	Wire.begin(SDA,SCL);

	// mandatory peripherals

//#ifdef ARDUINO_TBEAM_USE_RADIO_SX1262
	if(SetupPMIC())				{	Serial.println("PMIC Setup failed, halting ...\r\n");						while(1);				}
//#endif
	
	// SD card is optional but if present, modes of operation are configured
	// from a file rather than just compiled-in defaults.  It will also use
	// a more elaborate web page too
	
	if(SetupSDCard())			{	Serial.println("SD Card Setup failed, disabling ...\r\n");					sdcard_enable=0;		}

//#ifdef ARDUINO_TBEAM_USE_RADIO_SX1262
	if(SetupSPIFFS())			{	Serial.println("SPIFFS Setup failed, disabling ...\r\n");					spiffs_enable=0;		}
//#else
//	spiffs_enable=0;
//#endif
		
	ReadConfigFile();

	if(!sdcard_enable)		logging_enable=0;
	else					SetupLogging();
	
	SetupDisplay();
	
#if 0
	Serial.print("Hanging ...\r\n");
	while(1);
#endif	
#if 0
#ifdef ARDUINO_ARCH_ESP32
	if(SetupWiFi())				{	Serial.println("WiFi connection failed, disabling ...");					wifi_enable=0;			}
	if(SetupWebServer())		{	Serial.println("Web Server Setup failed, disabling ...");					webserver_enable=0;		}
#else
	wifi_enable=0;
	webserver_enable=0;
#endif
#endif

	if(SetupAccelerometer())	{	Serial.println("Accelerometer setup failed, disabling ...");				acc_enable=0;			}
	if(SetupGyro())				{	Serial.println("Gyro setup failed, disabling ...");							gyro_enable=0;			}
	if(SetupBarometer())		{	Serial.println("Barometer setup failed, disabling ...");					baro_enable=0;			}

	if(SetupLoRa())				{	Serial.println("LoRa Setup failed, halting ...\r\n");						while(1);				}
	if(SetupGPS())				{	Serial.println("GPS Setup failed, halting ...\r\n");						while(1);				}
	SetupOnePPS();
	if(SetupCrypto())			{	Serial.println("Crypto Setup failed, halting ...\r\n");						while(1);				}

#if 0
	Serial.println(crypto_key_hex);
	DumpHexPacket(crypto_key,32);
#endif
	
	if(SetupScheduler())		{	Serial.println("Scheduler Setup failed, halting ...\r\n");					while(1);				}

	// optional peripherals
	if(SetupLEDs())				{	Serial.println("LED Setup failed, halting ...\r\n");						leds_enable=0;			}

#if 0
	// optional peripherals	
	if(SetupBeeper())			{	Serial.println("Beeper Setup failed, disabling ...\r\n");					beeper_enable=0;		}
	if(SetupNeopixels())		{	Serial.println("Neopixels Setup failed, disabling ...\r\n");				neopixels_enable=0;		}
#endif
#if 0
	if(SetupTimers())			{	Serial.println("Timer Setup failed, falling back to software timing ...");	timer_enable=0;			}
#endif
}

int counter=0;

void loop()
{
	PollPMIC();
	PollSerial();
	PollGPS();
	PollOnePPS();
	PollLoRa();
	PollLEDs();
	PollDisplay();
	PollScheduler();
	
#if 0
	PollAccelerometer();
	PollGyro();
#endif
	
	PollBarometer();
}

void PollSerial(void)
{
	static uint8_t cmd[128];
	static uint16_t cmdptr=0;
	char rxbyte;

	while(Serial.available())
	{ 
		rxbyte=Serial.read();
		
		cmd[cmdptr++]=rxbyte;
		
		if((rxbyte=='\r')||(rxbyte=='\n'))
		{
			ProcessCommand(cmd,cmdptr);
			memset(cmd,0,sizeof(cmd));
			cmdptr=0;
		}
		else if(cmdptr>=sizeof(cmd))
		{
			cmdptr--;
		}
	}
}

void ProcessCommand(uint8_t *cmd,uint16_t cmdptr)
{
	int OK=0;
	
	switch(cmd[0]|0x20)
	{
		case 'a':	OK=AccelerometerCommandHandler(cmd,cmdptr);		break;
		case 'b':	OK=BarometerCommandHandler(cmd,cmdptr);			break;
		case 'y':	OK=GyroCommandHandler(cmd,cmdptr);				break;
		case 'g':	OK=GPSCommandHandler(cmd,cmdptr);				break;
		case 'l':	OK=LORACommandHandler(cmd,cmdptr);				break;
		case 'p':	OK=PMICCommandHandler(cmd,cmdptr);				break;
		case 'e':	OK=LEDCommandHandler(cmd,cmdptr);				break;
		case 'o':	OK=LongRangeCommandHandler(cmd,cmdptr);			break;
		case 'h':	OK=HighRateCommandHandler(cmd,cmdptr);			break;
		case 'n':	OK=NeopixelCommandHandler(cmd,cmdptr);			break;
		case 'z':	OK=BeeperCommandHandler(cmd,cmdptr);			break;
		
		case 'x':	OK=1;
					i2c_bus_scanner();
					break;
		
		case '?':	Serial.print("RocketTrack Test Harness Menu\r\n=================\r\n\n");
					Serial.print("g\t-\tGPS Commands\r\n");
					Serial.print("l\t-\tLoRa Commands\r\n");
					Serial.print("p\t-\tPMIC Commands\r\n");
					Serial.print("h\t-\tHigh Rate Mode Commands\r\n");
					Serial.print("o\t-\tLong Range Mode Commands\r\n");
					Serial.print("e\t-\tLed Commands\r\n");
					Serial.print("n\t-\tNeopixel Commands\r\n");
					Serial.print("b\t-\tBeeper Commands\r\n");
					Serial.print("t\t-\tTransmitter Mode\r\n");
					Serial.print("r\t-\tReceiver Mode\r\n");
					Serial.print("?\t-\tShow this menu\r\n");
					OK=1;
					break;
					
		default:	// do nothing
					break;
	}

	if(OK)	{	Serial.println("ok ...");	}
	else	{	Serial.println("?");	}
}

void i2c_bus_scanner(void)
{
	byte error, address;
	int nDevices;
	
	Serial.println("Scanning...");
	
	nDevices = 0;
	for(address = 1; address < 127; address++ ) 
	{
		// The i2c_scanner uses the return value of
		// the Write.endTransmisstion to see if
		// a device did acknowledge to the address.
		Wire.beginTransmission(address);
		error = Wire.endTransmission();
		
		if (error == 0)
		{
			Serial.print("I2C device found at address 0x");
			if (address<16) 
				Serial.print("0");
			Serial.print(address,HEX);
			Serial.println("  !");

			nDevices++;
		}
		else if (error==4) 
		{
			Serial.print("Unknown error at address 0x");
			if (address<16) 
				Serial.print("0");
			Serial.println(address,HEX);
		}    
	}
	
	if (nDevices == 0)
		Serial.println("No I2C devices found\n");
	else
		Serial.println("done\n");

	//	I2C device found at address 0x34  !		// AXP192 PMIC
	//	I2C device found at address 0x3C  !		// OLED Display
	//	I2C device found at address 0x68  !		// MPU6050 Accelerometer/Magnetometer/Gyro
	//	I2C device found at address 0x76  !		// BME280 pressure sensor  
}

bool IRAM_ATTR TinerHandler0(void *timerNo)
{



	return(true);
}

