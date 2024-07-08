
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

#ifdef ARDUINO_TBEAM_USE_RADIO_SX1262
	#include <axp20x.h>
#endif

#include <SPI.h>
#include <Wire.h>

#include "HardwareAbstraction.h"

#include "Accelerometer.h"
#include "Barometer.h"
#include "Beeper.h"
#include "CommandInterface.h"
#include "ConfigFile.h"
#include "Crypto.h"
#include "Display.h"
#include "GPS.h"
#include "Gyro.h"
#include "Leds.h"
#include "Logging.h"
#include "LoRaModule.h"
#include "Magnetometer.h"
#include "Neopixels.h"
#include "Scheduler.h"
#include "SDCard.h"
#include "SpiffsSupport.h"
#include "Timers.h"
#include "Webserver.h"
#include "WiFiSupport.h"

SemaphoreHandle_t i2c_mutex;

//extern char crypto_key_hex[65];
//extern uint8_t crypto_key[32];

char tracker_mode[32];
bool sync_sampling=true;

void setup()
{
	// Serial port
	Serial.begin(115200);
	
	Serial.print("\n--------\tRocketTrack Flight Telemetry System\t--------\r\n\n");

#ifndef ADAFRUIT_FEATHER_M0	
	SPI.begin(SCK,MISO,MOSI);
#else
	SPI.begin();
#endif

	Wire.begin(SDA,SCL);
	Wire.setClock(400000);

#if USE_FREERTOS
	i2c_mutex=xSemaphoreCreateMutex();
	if(i2c_mutex==NULL)
	{
		Serial.println("Failed to create I2C mutex.  This is generally bad ...");
	}
#endif

	// mandatory peripherals

	SetupCommandHandler();

#ifdef ARDUINO_TBEAM_USE_RADIO_SX1262
	if(SetupPMIC())				{	Serial.println("PMIC Setup failed, halting ...\r\n");						while(1);				}
#endif
	
	// SD card is optional but if present, modes of operation are configured
	// from a file rather than just compiled-in defaults.  It will also use
	// a more elaborate web page too
	
#if 0
	if(SetupSDCard())			{	Serial.println("SD Card Setup failed, disabling ...\r\n");					sdcard_enable=false;	}
#endif

	if(SetupSPIFFS())			{	Serial.println("SPIFFS Setup failed, disabling ...\r\n");					spiffs_enable=false;	}

#if 0
	Serial.println("Hanging for now ...");
	while(1)
	{
		Serial.print(".");
		delay(1000);
	}
#endif
		
	ReadConfigFile();

#if 0
	if(!sdcard_enable)		logging_enable=false;
	else					SetupLogging();
#endif

	SetupDisplay();

#if 0	
	#ifdef ARDUINO_ARCH_ESP32
		if(SetupWiFi())				{	Serial.println("WiFi connection failed, disabling ...");					wifi_enable=false;		}
		if(SetupWebServer())		{	Serial.println("Web Server Setup failed, disabling ...");					webserver_enable=false;	}
		#else
		wifi_enable=false;
		webserver_enable=false;
	#endif
#endif

#if 0
	acc_enable=false;
	baro_enable=false;
	gyro_enable=false;
	mag_enable=false;
#endif

	if(acc_enable&&SetupAccelerometer())	{	Serial.println("Accelerometer setup failed, disabling ...");	acc_enable=false;		}
	if(gyro_enable&&SetupGyro())			{	Serial.println("Gyro setup failed, disabling ...");				gyro_enable=false;		}
	if(mag_enable&&SetupMagnetometer())		{	Serial.println("Magnetometer setup failed, disabling ...");		mag_enable=false;		}
	if(baro_enable&&SetupBarometer())		{	Serial.println("Barometer setup failed, disabling ...");		baro_enable=false;		}
	
	if(SetupGPS())				{	Serial.println("GPS Setup failed, halting ...\r\n");						while(1);				}
//	SetupOnePPS();

	if(SetupLoRa())				{	Serial.println("LoRa Setup failed, halting ...\r\n");						while(1);				}
	if(SetupCrypto())			{	Serial.println("Crypto Setup failed, halting ...\r\n");						while(1);				}

	#if 0
		Serial.println(crypto_key_hex);	DumpHexPacket(crypto_key,32,true);
	#endif

	if(SetupScheduler())		{	Serial.println("Scheduler Setup failed, halting ...\r\n");					while(1);				}

	if(SetupLEDs())				{	Serial.println("LED Setup failed, halting ...\r\n");						leds_enable=false;		}

	// optional peripherals
#if 0
	if(SetupBeeper())			{	Serial.println("Beeper Setup failed, disabling ...\r\n");					beeper_enable=false;	}
#endif
#if 0
	if(SetupNeopixels())		{	Serial.println("Neopixels Setup failed, disabling ...\r\n");				neopixels_enable=false;	}
#endif
#if 0
	if(SetupTimers())			{	Serial.println("Timer Setup failed, falling back to software timing ...");	timer_enable=false;		}
#endif

	Serial.println(baro_enable?"Barometer enabled":"Barometer disabled");
	Serial.println(acc_enable?"Accelerometer enabled":"Accelerometer disabled");
	Serial.println(gyro_enable?"Gyro enabled":"Gyro disabled");
	Serial.println(mag_enable?"Magnetometer enabled":"Magnetometer disabled");
	Serial.println("");
}

int counter=0;

void loop()
{
	PollSerial();

#ifndef USE_FREERTOS
	PollGPS();
#endif
}

#ifndef ADAFRUIT_FEATHER_M0

// this is for using hardware timers on ESP32 processors.  I can't quite
// remember why I wanted to do that now so this may well get deleted in the
// near future

bool IRAM_ATTR TinerHandler0(void *timerNo)
{
	return(true);
}

#endif

