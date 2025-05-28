
#define DEBUGCONFIG 0
  
#include "Barometer.h"
#include "Crypto.h"
#include "FlightEvents.h"
#include "GPS.h"
#include "IMU.h"
#include "Logging.h"
#include "LoRaModule.h"
#include "Magnetometer.h"
#include "SDCard.h"
#include "Webserver.h"

#include <IniFile.h>
#include <SPIFFSIniFile.h>

enum
{
	CFGSTRING=0,
	CFGINTEGER,
	CFGFLOAT,
	CFGIPADDRESS
};

enum
{
	WIFI_STATION_MODE=0,
	WIFI_AP_MODE
};

typedef struct
{
	const char section[32];
	const char tag[32];
	void *variable;
	int type;
	const char defaultvalue[32];
} configvalue_t;

configvalue_t config[]={
	{	"Tracker",			"Autostart",		(void *)&lora_constant_transmit,	CFGINTEGER,		"0"							},
	{	"Crypto",			"Enable",			(void *)&crypto_enable,				CFGINTEGER,		"1"							},
	{	"Crypto",			"Key",				(void *)crypto_key_hex,				CFGSTRING,		""							},
	{	"WiFi",				"Enable",			(void *)&wifi_enable,				CFGINTEGER,		"1"							},
	{	"WiFi",				"SSID",				(void *)ssid,						CFGSTRING,		"MOLEY"						},
	{	"WiFi",				"Password",			(void *)password,					CFGSTRING,		"sausageeggchipsandbeans"	},
	{	"WiFi",				"APSSID",			(void *)apssid,						CFGSTRING,		"Rocket"					},
	{	"WiFi",				"APPassword",		(void *)appassword,					CFGSTRING,		"eggsbenedict"				},
	{	"WiFi",				"Mode",				(void *)&wifi_mode,					CFGINTEGER,		WIFI_STATION_MODE			},
	{	"WiFi",				"IPAddress",		(void *)&LocalIP,					CFGIPADDRESS,	"192.168.0.170"				},
	{	"WiFi",				"Subnet",			(void *)&Subnet,					CFGIPADDRESS,	"255.255.255.0"				},
	{	"WiFi",				"Gateway",			(void *)&Gateway,					CFGIPADDRESS,	"192.168.0.254"				},
	{	"WiFi",				"DNS1",				(void *)&DNS1,						CFGIPADDRESS,	"212.23.3.100"				},
	{	"WiFi",				"DNS2",				(void *)&DNS2,						CFGIPADDRESS,	"212.23.6.100"				},
	{	"LoRa",				"Frequency",		(void *)&lora_freq,					CFGFLOAT,		"434.650"					},
	{	"LoRa",				"Mode",				(void *)lora_mode,					CFGSTRING,		"High Rate"					},
	{	"LoRa",				"EnableCRC",		(void *)&lora_crc,					CFGINTEGER,		"1"							},
	{	"High Rate",		"Bandwidth",		(void *)&hr_bw,						CFGINTEGER,		"125000"					},
	{	"High Rate",		"SpreadingFactor",	(void *)&hr_sf,						CFGINTEGER,		"7"							},
	{	"High Rate",		"CodingRate",		(void *)&hr_cr,						CFGINTEGER,		"8"							},
	{	"High Rate",		"TransmitPeriodMS",	(void *)&hr_period,					CFGINTEGER,		"1000"						},
	{	"Long Range",		"Bandwidth",		(void *)&lr_bw,						CFGINTEGER,		"31250"						},
	{	"Long Range",		"SpreadingFactor",	(void *)&lr_sf,						CFGINTEGER,		"12"						},
	{	"Long Range",		"CodingRate",		(void *)&lr_cr,						CFGINTEGER,		"8"							},
	{	"Long Range",		"TransmitPeriodMS",	(void *)&lr_period,					CFGINTEGER,		"10000"						},
	{	"Flight Events",	"DetectLaunch",		(void *)&detect_launch,				CFGINTEGER,		"1"							},
	{	"Flight Events",	"DetectApogee",		(void *)&detect_apogee,				CFGINTEGER,		"1"							},
	{	"Flight Events",	"DetectLanding",	(void *)&detect_landing,			CFGINTEGER,		"1"							},
	{	"GPS",				"Type",				(void *)gps_type,					CFGSTRING,		"Generic"					},
	{	"GPS",				"InitialBaudRate",	(void *)&initial_baud,				CFGINTEGER,		"9600"						},
	{	"GPS",				"FinalBaudRate",	(void *)&final_baud,				CFGINTEGER,		"115200"					},
	{	"GPS",				"FixRate",			(void *)&fix_rate,					CFGINTEGER,		"5"							},
	{	"Barometer",		"Enable",			(void *)&baro_enable,				CFGINTEGER,		"1"							},
	{	"Barometer",		"Type",				(void *)baro_type,					CFGSTRING,		"BME280"					},
	{	"Barometer",		"MeasurementRate",	(void *)&baro_rate,					CFGINTEGER,		"100"						},
	{	"Barometer",		"GPSSync",			(void *)&baro_gps_sync,				CFGINTEGER,		"1"							},
//	{	"Accelerometer",	"Enable",			(void *)&acc_enable,				CFGINTEGER,		"1"							},
//	{	"Accelerometer",	"Type",				(void *)acc_type,					CFGSTRING,		"MPU6050"					},
//	{	"Accelerometer",	"MeasurementRate",	(void *)&acc_rate,					CFGINTEGER,		"200"						},
//	{	"Gyro",				"Enable",			(void *)&gyro_enable,				CFGINTEGER,		"1"							},
//	{	"Gyro",				"Type",				(void *)gyro_type,					CFGSTRING,		"MPU6050"					},
//	{	"Gyro",				"MeasurementRate",	(void *)&gyro_rate,					CFGINTEGER,		"200"						},
//	{	"Magnetometer",		"Enable",			(void *)&mag_enable,				CFGINTEGER,		"1"							},
//	{	"Magnetometer",		"Type",				(void *)mag_type,					CFGSTRING,		"None"						},
//	{	"Magnetometer",		"MeasurementRate",	(void *)&mag_rate,					CFGINTEGER,		"200"						},
	{	"Logging",			"Level",			(void *)&log_level,					CFGINTEGER,		"1"							},
	{	"",					"",					0,									0,				""							}
};

void printErrorMessage(uint8_t e,bool eol=true)
{
	switch(e)
	{
		case IniFile::errorNoError:			Serial.print("no error");				break;
		case IniFile::errorFileNotFound:	Serial.print("file not found");			break;
		case IniFile::errorFileNotOpen:		Serial.print("file not open");			break;
		case IniFile::errorBufferTooSmall:	Serial.print("buffer too small");		break;
		case IniFile::errorSeekError:		Serial.print("seek error");				break;
		case IniFile::errorSectionNotFound:	Serial.print("section not found");		break;
		case IniFile::errorKeyNotFound:		Serial.print("key not found");			break;
		case IniFile::errorEndOfFile:		Serial.print("end of file");			break;
		case IniFile::errorUnknownError:	Serial.print("unknown error");			break;

		default:							Serial.print("unknown error value");	break;
	}
  
	if(eol)
    	Serial.println();
}

int ReadConfigFileSDCard(void)
{
	const size_t bufferLen=128;
	char buffer[bufferLen];
	const char *filename="/tracker.ini";

	IniFile ini(filename);
		
	if(!ini.open())
	{
		Serial.print("\tIni file ");	Serial.print(filename);		Serial.println(" does not exist");
		
		// Cannot do anything else
		return(1);
	}
	
	Serial.println("Ini file exists");
	
	// Check the file is valid. This can be used to warn if any lines
	// are longer than the buffer.
	
	if(!ini.validate(buffer,bufferLen))
	{
		Serial.print("\tini file ");	Serial.print(ini.getFilename());	Serial.print(" not valid: ");	printErrorMessage(ini.getError());
		
		// Cannot do anything else
		return(1);
	}
}

int ReadConfigFileSPIFFS(void)
{
	const size_t bufferLen=128;
	char buffer[bufferLen];
	const char *filename="/tracker.ini";

	SPIFFSIniFile ini(filename);
		
	if(!ini.open())
	{
		Serial.print("\tIni file ");	Serial.print(filename);		Serial.println(" does not exist");
			
		// Cannot do anything else
		return(1);
	}
	
	Serial.println("\tIni file exists");
	
	// Check the file is valid. This can be used to warn if any lines
	// are longer than the buffer.
	
	if(!ini.validate(buffer,bufferLen))
	{
		Serial.print("\tini file ");	Serial.print(ini.getFilename());	Serial.print(" not valid: ");	printErrorMessage(ini.getError());
		
		// Cannot do anything else
		return(1);
	}

	Serial.print("\tini file is valid, starting to read the parameters\r\n");
	
	int cnt=0;
	int values_used=0;
	while(1)
	{
		if(		strlen(config[cnt].section)==0
			&&	strlen(config[cnt].tag)==0		)
		{
			break;
		}
		
#if DEBUGCONFIG
		Serial.print("\t\tLooking for Tag \"");		Serial.print(config[cnt].tag);	Serial.print("\" in Section \"");	Serial.print(config[cnt].section);	Serial.print("\"");
#endif
		
		if(ini.getValue(config[cnt].section,config[cnt].tag,buffer,sizeof(buffer)))
		{
#if DEBUGCONFIG
			Serial.print("\t-\tvalue = ");			Serial.println(buffer);
#endif
		}
		else
		{
			Serial.print(" Tag \"");		Serial.print(config[cnt].tag);	Serial.print("\" in Section \"");	Serial.print(config[cnt].section);	Serial.print("\"\t-\tnot found, subsituting the default value");
			strcpy(buffer,config[cnt].defaultvalue);
		}
		
		if(config[cnt].variable!=NULL)
		{
			if(config[cnt].type==CFGSTRING)
			{
				strcpy((char *)config[cnt].variable,buffer);
			}
			else if(config[cnt].type==CFGIPADDRESS)	
			{
				IPAddress temp;
				temp.fromString(buffer);
				*((IPAddress *)config[cnt].variable)=temp;
			}
			else if(config[cnt].type==CFGINTEGER)	{	*((int *)config[cnt].variable)=atoi(buffer);				}
			else if(config[cnt].type==CFGFLOAT)		{	*((double *)config[cnt].variable)=atof(buffer);				}
			else									{	Serial.println("Unknown data type requested ...");			}
		}
		
		cnt++;
	}
	
	Serial.println("Config file read ...");
	
	return(0);
}

void ReadConfigFile(void)
{
	int sdcard_read_error=0;
	int spiffs_read_error=0;

#if 0
	if(sdcard_enable)
	{
		Serial.println("\tAttempting to read the config from an SD card");
		sdcard_read_error=ReadConfigFileSDCard();
	}	
	else
		sdcard_read_error=1;
	
	if(sdcard_read_error)
	{
		Serial.println("\tReading the config from an SD card failed ...");

		if(spiffs_enable)
		{
#endif
			Serial.println("\tAttempting to read the config from SPI Flash");
			spiffs_read_error=ReadConfigFileSPIFFS();
			
			if(spiffs_read_error)
				Serial.println("\tReading the config from SPI Flash failed ...");
#if 0
		}
		
		if(sdcard_read_error&&spiffs_read_error)
		{
			Serial.println("\tUsing the default config values");
			SetDefaultConfigValues();
		}
	}
#endif
}

void SetDefaultConfigValues(void)
{
  
}

