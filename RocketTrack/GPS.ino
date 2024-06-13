
#include "Accelerometer.h"
#include "Barometer.h"
#include "GPS.h"
#include "Gyro.h"
#include "Magnetometer.h"
#include "Packetisation.h"
#include "SensorState.h"

#include <Adafruit_GPS.h>

#include <TinyGPS.h>
#include <MicroNMEA.h>

//MicroNMEA library structures
char gpsparserBuffer[100];
MicroNMEA gpsparser(gpsparserBuffer,sizeof(gpsparserBuffer));

//TinyGPS gpsparser;

bool gps_enabled=true;
int gps_type_num=GPS_NMEA;

// pick up the right serial port to use depending on which board we're running on

#if ARDUINO_TBEAM_USE_RADIO_SX1276

	#define GPSSerialPort Serial1
	
#elif ARDUINO_TTGO_LoRa32_v21new

	#include <SoftwareSerial.h>
//	EspSoftwareSerial::UART GPSSerialPort;

	Adafruit_GPS gpsi2c(&Wire);
	
	
	#include "MTK_GPS.h"
//	MTK_GPS gps(GPSSerialPort);

#elif BOARD_FEATHER
	
	#define GPSSerialPort NULL // for now
	
#else
	
	#define GPSSerialPort NULL
	
#endif

// to be set from the config file

char gps_type[32];
int initial_baud;
int final_baud;
int fix_rate;

#define DEBUG 1

#define GPS_PASSTHROUGH 0
#define BINARY_GPS		1

#define MAX_CHANNELS 50

// Globals
byte RequiredFlightMode=0;
byte GlonassMode=0;
byte RequiredPowerMode=-1;
byte LastCommand1=0;
byte LastCommand2=0;
byte HaveHadALock=0;

bool gps_live_mode=0;

// these are all unpacked from UBX messages

// from NAV-STATUS
uint32_t iTOW=0;
uint8_t gpsFix=0;
uint8_t flags=0;
uint8_t fixStat=0;
uint8_t flags2=0;
uint32_t ttff=0;
uint32_t msss=0;

// from NAV-SVINFO
uint8_t beaconnumCh=0;
uint8_t globalFlags=0;
uint16_t reserved2=0;

uint8_t chn[MAX_CHANNELS];
uint8_t svid[MAX_CHANNELS];
uint8_t svflags[MAX_CHANNELS];
uint8_t quality[MAX_CHANNELS];
uint8_t cno[MAX_CHANNELS];
int8_t elev[MAX_CHANNELS];
int16_t azim[MAX_CHANNELS];
int32_t prRes[MAX_CHANNELS];

uint8_t beaconnumSats=0;

// from NAV-POSLLH
//int32_t gps_lon=0;
//int32_t gps_lat=0;
//int32_t gps_height=0;
//int32_t gps_hMSL=0;
//int32_t max_beaconhMSL=0;
//uint32_t gps_hAcc=0;
//uint32_t gps_vAcc=0;

//uint8_t gps_hAccValue=0;

// from NAV-TIMEUTC

uint16_t beaconyear;
uint8_t beaconmonth;
uint8_t beaconday;
uint8_t beaconhour;
uint8_t beaconmin;
uint8_t beaconsec;

int SetupGPS(void)
{
	Serial.println("Open GPS port");
	
	// this could do with some autobauding
	
	

	// this also assumes we're using a u-Blox receiver, not always true



#if ARDUINO_TBEAM_USE_RADIO_SX1276
	// Pins for T-Beam v0.8 (3 push buttons) and up
	GPSSerialPort.begin(initial_baud,SERIAL_8N1,34,12);
#elif ARDUINO_TTGO_LoRa32_v21new
//	GPSSerialPort.begin(initial_baud,EspSoftwareSerial::SWSERIAL_8N1,GPS_RXD,GPS_TXD,false,95,11);

	gpsi2c.begin(0x10);
	
#elif BOARD_FEATHER
	Serial.println("GPS support not present for the Feather board yet, aborting ...");
	return(1);
#else
	Serial.println("GPS misconfigured, aborting ...");
	return(1);
#endif
#if 0
	Serial.print("\nPassing through the GPS for 10 seconds ...\r\n\n");

	int start=millis();
	
	while(millis()<(start+10000))
	{
		while(GPSSerialPort.available()>0)
		{
			Serial.write(GPSSerialPort.read());
		}
	}

	Serial.print("\r\n\nDisabling GPS passthrough\r\n\n");
#endif
	
	if(strstr("NMEA",gps_type)!=NULL)			{	gps_type_num=GPS_NMEA;		}
	else if(strstr("UBLOX",gps_type)!=NULL)		{	gps_type_num=GPS_UBLOX;		}
	else if(strstr("ublox",gps_type)!=NULL)		{	gps_type_num=GPS_UBLOX;		}
	else if(strstr("MTK333X",gps_type)!=NULL)	{	gps_type_num=GPS_MTK333x;	}
	else if(strstr("mtk333x",gps_type)!=NULL)	{	gps_type_num=GPS_MTK333x;	}
	else if(strstr("MTK3333",gps_type)!=NULL)	{	gps_type_num=GPS_MTK333x;	}
	else if(strstr("mtk3333",gps_type)!=NULL)	{	gps_type_num=GPS_MTK333x;	}
	else if(strstr("MTK3339",gps_type)!=NULL)	{	gps_type_num=GPS_MTK333x;	}
	else if(strstr("mtk3339",gps_type)!=NULL)	{	gps_type_num=GPS_MTK333x;	}
	else										{	gps_enabled=false;			}
	
	SetupGPSMessages();

	return(0);
}

void SetupGPSMessages(void)
{
	switch(gps_type_num)
	{
		case GPS_UBLOX:		Serial.println("\tUBLOX GPS Receiver selected\n");
#if 0
							// turn off all NMEA output	
							SetMessageRate(0xf0,0x00,0x00);	// GPGGA
							SetMessageRate(0xf0,0x01,0x00);	// GPGLL
							SetMessageRate(0xf0,0x02,0x00);	// GPGSA
							SetMessageRate(0xf0,0x03,0x00);	// GPGSV
							SetMessageRate(0xf0,0x04,0x00);	// GPRMC
							SetMessageRate(0xf0,0x05,0x00);	// GPVTG
							
							// change the baud rate and re-open the
							// serial port to match
							ChangeBaudRate(115200);
							GPSSerialPort.flush();
							GPSSerialPort.end();
							Serial1.begin(115200,SERIAL_8N1,34,12);	// Pins for T-Beam v0.8 (3 push buttons) and up

							// setup the messages we want
							SetMessageRate(0x01,0x02,0x01);	// NAV-POSLLH every fix
							SetMessageRate(0x01,0x03,0x05);	// NAV-STATUS every fifth fix
							SetMessageRate(0x01,0x30,0x05);	// NAV-SVINFO every fifth fix
							SetMessageRate(0x01,0x21,0x05);	// NAV-TIMEUTC every fifth fix
							
							Set5Hz_Fix_Rate();
#endif						
							break;
						
		case GPS_MTK333x:	Serial.println("\tMTX333X GPS Receiver selected\n");	
		
							break;
	
		default:			// assume this is just a generic GPS and leave the
							// messages it output as is.
							Serial.println("\tConfiguring as a standard GPS, no custom messages selected");
							break;
	}
}

void PollGPS(void)
{
//	static char gpsbuffer[256];
//	static int bufferptr=0;
	char rxbyte;
	
#if GPS_PASSTHROUGH
	if(GPSSerialPort.available())	{	rxbyte=GPSSerialPort.read();	Serial.write(rxbyte);			}
	if(Serial.available())			{	rxbyte=Serial.read();			GPSSerialPort.write(rxbyte);	}
#else

//	while(GPSSerialPort.available())
	if(gpsi2c.available())
	{
//		rxbyte=GPSSerialPort.read();
		rxbyte=gpsi2c.read();
		
//		gpsbuffer[bufferptr++]=rxbyte;
//		if(bufferptr>=sizeof(gpsbuffer))	bufferptr=0;
		
		if(gps_live_mode)
			Serial.write(rxbyte);

//		if(gpsparser.process(rxbyte))
//		{
//			Serial.println(gpsparser.getSentence());
//		}
	}

#if 0		
		if(		(gps_type_num==GPS_NMEA)
			||	(gps_type_num==GPS_MTK333x)	)
		{
			if(gpsparser.process(rxbyte))
			{
#if 0
				Serial.println(gpsbuffer);
				memset(gpsbuffer,0,sizeof(gpsbuffer));
#endif
				
				Serial.println(gpsparser.getSentence());
				
				if(strncmp(gpsparser.getMessageID(),"GGA",3)==0)
				{
					if(sync_sampling)
					{
						// sample all the sensor at the same time as the GGA message is received

						int starttime=micros();

//						trigger_baro=true;
//						trigger_accel=true;
//						trigger_gyro=true;
//						trigger_mag=true;
						
#if 0
						ss.baro_altitude=ReadAltitude();
						ss.baro_pressure=ReadPressure();
						ss.baro_temperature=ReadTemperature();
						ss.baro_humidity=ReadHumidity();
#endif
#if 0
						ReadAccelerometer(&ss.accel_x,&ss.accel_y,&ss.accel_z);
#endif
#if 0
						ReadGyro(&ss.gyro_x,&ss.gyro_y,&ss.gyro_z);
#endif
#if 0
						ReadMagnetometer(&ss.mag_x,&ss.mag_y,&ss.mag_z);
#endif
						Serial.println("Log ...");
#if 0
						char buffer[256];
						sprintf(buffer,"AccX: %.2f, AccY: %.2f, AccZ: %.2f, GyroX: %.2f, GyroY: %.2f, GyroZ: %.2f, MagX: %.2f, MagY: %.2f, MagZ: %.2f, Alt: %.2f, Pres: %.2f, Temp: %.2f, Hum: %.2f",
									ss.accel_x,ss.accel_y,ss.accel_z,
									ss.gyro_x,ss.gyro_y,ss.gyro_z,
									ss.mag_x,ss.mag_y,ss.mag_z,
									ss.baro_altitude,ss.baro_pressure,ss.baro_temperature,ss.baro_humidity);
						
						Serial.println(buffer);
#endif

						Serial.print("Duration: ");	Serial.print(micros()-starttime);	Serial.println(" uS");
					}
				}

				if(strncmp(gpsparser.getMessageID(),"RMC",3)==0)
				{	
	#if 0
					// Output GPS information from previous second
					Serial.print("Valid fix: ");		Serial.println(gpsparser.isValid() ? "yes" : "no");

					Serial.print("Nav. system: ");
					if(gpsparser.getNavSystem())		Serial.println(gpsparser.getNavSystem());
					else								Serial.println("none");

					Serial.print("Num. satellites: ");	Serial.println(gpsparser.getNumSatellites());
					Serial.print("HDOP: ");				Serial.println(gpsparser.getHDOP()/10., 1);

					Serial.print("Date/time: ");		Serial.print(gpsparser.getYear());		Serial.print('-');	Serial.print(int(gpsparser.getMonth()));	Serial.print('-');	Serial.print(int(gpsparser.getDay()));
					Serial.print('T');					Serial.print(int(gpsparser.getHour()));	Serial.print(':');	Serial.print(int(gpsparser.getMinute()));	Serial.print(':');	Serial.println(int(gpsparser.getSecond()));

					long latitude_mdeg = gpsparser.getLatitude();
					long longitude_mdeg = gpsparser.getLongitude();

					Serial.print("Latitude (deg): ");	Serial.println(latitude_mdeg / 1000000., 6);
					Serial.print("Longitude (deg): ");	Serial.println(longitude_mdeg / 1000000., 6);

					long alt;
					
					Serial.print("Altitude (m): ");
					if(gpsparser.getAltitude(alt))		Serial.println(alt / 1000., 3);
					else								Serial.println("not available");

					Serial.print("Speed: ");			Serial.println(gpsparser.getSpeed() / 1000., 3);
					Serial.print("Course: ");			Serial.println(gpsparser.getCourse() / 1000., 3);

					Serial.println("-----------------------");
	#endif
				}
			}
		}
	}
#endif
#endif
}

int GPSCommandHandler(uint8_t *cmd,uint16_t cmdptr)
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
		case 'p':	// position fix
					Serial.printf("Lat = %.6f, Lon = %.6f, ",beaconlat/1e7,beaconlon/1e7);
					Serial.printf("height = %.1f\r\n",beaconheight/1e3);
					break;
		
		case 'f':	// fix status
					if(gpsFix==0x00)		Serial.println("No Fix");
					else if(gpsFix==0x02)	Serial.println("2D Fix");
					else if(gpsFix==0x03)	Serial.println("3D Fix");
					break;
		
		case 's':	// satellite info
					Serial.println("Chan\tPRN\tElev\tAzim\tC/No");
					for(cnt=0;cnt<beaconnumCh;cnt++)
					{
						Serial.print(cnt);	Serial.print("\t"); Serial.print(svid[cnt]);	Serial.print("\t");	Serial.print(elev[cnt]);	Serial.print("\t");	Serial.print(azim[cnt]);	Serial.print("\t");	Serial.println(cno[cnt]);
					}
					
					break;
		
		case 'l':	// live mode toggle
					gps_live_mode=!gps_live_mode;
					break;
		
		case '?':	Serial.print("GPS Test Harness\r\n================\r\n\n");
					Serial.print("p\t-\tCheck positon\r\n");
					Serial.print("f\t-\tCheck fix status\r\n");
					Serial.print("s\t-\tCheck satellite status\r\n");
					Serial.print("l\t-\tLive GPS data on/off\r\n");
					Serial.print("?\t-\tShow this menu\r\n");
					break;
		
		default:	retval=0;
					break;
	}
	
	return(retval);
}
