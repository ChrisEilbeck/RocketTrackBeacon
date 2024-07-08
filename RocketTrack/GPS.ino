
#include "Accelerometer.h"
#include "Barometer.h"
#include "GPS.h"
#include "Gyro.h"
#include "HardwareAbstraction.h"
#include "Magnetometer.h"
#include "Packetisation.h"
#include "SensorState.h"


#include <Adafruit_GPS.h>
//Adafruit_GPS gpsparser(&Wire);

//#include <TinyGPS.h>
//TinyGPS gpsparser;

//#include <MicroNMEA.h>
//MicroNMEA library structures
//char gpsparserBuffer[100];
//MicroNMEA gpsparser(gpsparserBuffer,sizeof(gpsparserBuffer));

//#include <TinyGPSPlus.h>
//TinyGPSPlus gpsparser;

//#include <ArduinoNmeaParser.h>
//void onRmcUpdate(nmea::RmcData const);
//void onGgaUpdate(nmea::GgaData const);
//ArduinoNmeaParser gpsparser(onRmcUpdate,onGgaUpdate);

bool gps_enabled=true;
int gps_type_num=GPS_NMEA;

#define MAX_MESSAGE_LENGTH 80

QueueHandle_t QueueHandle;
const int QueueElementSize=8;
typedef struct {
	uint8_t count;
	char rxbytes[MAX_MESSAGE_LENGTH+1];
} message_t;

// pick up the right serial port to use depending on which board we're running on

#if ARDUINO_TBEAM_USE_RADIO_SX1276

	#define GPSSerialPort Serial1
	
#elif ARDUINO_TTGO_LoRa32_v21new

	#include <SoftwareSerial.h>
//	EspSoftwareSerial::UART GPSSerialPort;
	Adafruit_GPS gpsi2c(&Wire);
	
	#define GPSSerialPort gpsi2c
	
//	#include "MTK_GPS.h"
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

bool gps_live_mode=false;

int SetupGPS(void)
{
	Serial.println("Open GPS port");
	
	// this could do with some autobauding
	
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
	



#if ARDUINO_TBEAM_USE_RADIO_SX1276

	// Pins for T-Beam v0.8 (3 push buttons) and up
	GPSSerialPort.begin(initial_baud,SERIAL_8N1,34,12);

#elif ARDUINO_TTGO_LoRa32_v21new

	#if 0
		Serial.println("Setting up for SoftSerial GPS at address");
		GPSSerialPort.begin(initial_baud,EspSoftwareSerial::SWSERIAL_8N1,GPS_RXD,GPS_TXD,false,95,11);
	#else
		Serial.println("Setting up for I2C GPS at address 0x10");
		GPSSerialPort.begin(0x10);
	#endif
	
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
		
	SetupGPSMessages();

#ifdef USE_FREERTOS
	xTaskCreatePinnedToCore(GPSReceiveTask,"GPS Input Task",20480,NULL,2,NULL,0);
#endif
	
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
							
							// set up for standard messages initially,
							// GxGGA, GxRMC, GxGSA, GxGSV etc.
							
							
							
							
							break;
	
		default:			// assume this is just a generic GPS and leave the
							// messages it output as is.
							Serial.println("\tConfiguring as a standard GPS, no custom messages selected");
							break;
	}
}

void ReconfigureGPSMessages(void)
{



}

void GPSReceiveTask(void *pvParameters)
{
	char rxbyte;
	char buffer[256];
	int bufferptr=0;
	int last_poor_fix=millis();
	bool gps_reconfigured=false;
	
	while(1)
	{
		xSemaphoreTake(i2c_mutex,portMAX_DELAY);

		if(GPSSerialPort.available())
		{
			rxbyte=GPSSerialPort.read();
			xSemaphoreGive(i2c_mutex);

			if(gps_live_mode)
				Serial.write(rxbyte);

			if(GPSSerialPort.newNMEAreceived())
			{
				GPSSerialPort.parse(GPSSerialPort.lastNMEA());
				
				if(!gps_reconfigured)
				{
					if(		(GPSSerialPort.fixquality_3d==3)
						&&	(GPSSerialPort.satellites>=5)
						&&	((millis()-last_poor_fix)>=5000)		)
					{
						// we have had a good 3d fix with at least 5 seconds so
						// reconfigure the GPS for high rate measurements and
						// only outputting GxGGA messages
						
						ReconfigureGPSMessages();
					}
					else
						last_poor_fix=millis();
				}
				
				ss.gps_latitude=GPSSerialPort.latitudeDegrees;
				ss.gps_longitude=GPSSerialPort.longitudeDegrees;
				ss.gps_altitude=GPSSerialPort.altitude;
				
				if(ss.gps_max_altitude<ss.gps_altitude)
					ss.gps_max_altitude=ss.gps_altitude;				
			
#if 0	
				Serial.print(GPSSerialPort.latitudeDegrees,6);
				Serial.print(", ");
				Serial.print(GPSSerialPort.longitudeDegrees,6);
				Serial.print(", ");
				Serial.println(GPSSerialPort.altitude,1);
#endif
			}

			// could potentially ditch this and just tag sampling the other
			// sensors into the above new NMEA routine
			
			static int state=0;			
			switch(state)
			{
				case 0:		if(rxbyte=='G')	state=1;
							break;
				
				case 1:		state=2;
							break;
				
				case 2:		if(rxbyte=='G')	state=3;
							else			state=0;
							break;
				
				case 3:		if(rxbyte=='G')	state=4;
							else			state=0;
							break;
				
				case 4:		if(rxbyte=='A') state=5;
							else			state=0;
							break;
							
				case 5:		if(rxbyte=='\n')
							{
//								Serial.println("GxGGA ...");									
								baro_trigger=true;
//								delay(1);
							}
							
							if(rxbyte=='$')	state=0;
							
							break;
				
				default:	state=0;
							break;
			}
		}
		else
		{
			xSemaphoreGive(i2c_mutex);
		}
		
		delay(1);
	}
}

void PollGPS(void)
{
	char rxbyte;
	
	if(GPSSerialPort.available())
	{
		char rxbyte=GPSSerialPort.read();
		
		if(gps_live_mode)
			Serial.write(rxbyte);
	}
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
		case 'r':	// reset gps
					Serial.println("GPS full reset");
					GPSSerialPort.println("$PMTK104*37");
					break;
		
		case 'c':	// cold start
					Serial.println("GPS cold start");
					GPSSerialPort.println("$PMTK103*30");
					break;
		
		case 'w':	// warm start
					Serial.println("GPS warm start");
					GPSSerialPort.println("$PMTK102*31");
					break;
		
		case 'h':	// hot start
					Serial.println("GPS hot start");
					GPSSerialPort.println("$PMTK101*32");
					break;
	
		case 't':	// time
					Serial.println("GPS time");
#if 0
					Serial.print(gpsparser.date.day());		Serial.print("/");	Serial.print(gpsparser.date.month());	Serial.print("/");	Serial.print(gpsparser.date.year());	Serial.print(" ");
					Serial.print(gpsparser.time.hour());	Serial.print(":");	Serial.print(gpsparser.time.minute());	Serial.print(":");	Serial.println(gpsparser.time.second());
#endif
					break;
		
		case 'p':	// position fix
					Serial.println("GPS position");
#if 0
				    Serial.print(gpsparser.location.lat(),6);
					Serial.print(", ");
				    Serial.print(gpsparser.location.lng(),6);
					Serial.print(", ");
					Serial.print(gpsparser.altitude.meters(),1);
					Serial.println(" m");
#endif
					break;
		
		case 'f':	// fix status
					Serial.println("GPS fix status");
//					Serial.print("Fix: ");		Serial.print((int)GPSSerialPort.fix);
//					Serial.print(" quality: ");	Serial.println((int)GPSSerialPort.fixquality);

#if 0
					if(ss.gps_fix==0x00)		Serial.println("No Fix");
					else if(ss.gps_fix==0x02)	Serial.println("2D Fix");
					else if(ss.gps_fix==0x03)	Serial.println("3D Fix");
#endif
					break;

#if 0		
		case 's':	// satellite info
					Serial.println("Chan\tPRN\tElev\tAzim\tC/No");
					for(cnt=0;cnt<beaconnumCh;cnt++)
					{
						Serial.print(cnt);	Serial.print("\t"); Serial.print(svid[cnt]);	Serial.print("\t");	Serial.print(elev[cnt]);	Serial.print("\t");	Serial.print(azim[cnt]);	Serial.print("\t");	Serial.println(cno[cnt]);
					}
					
					break;
#endif
				
		case 'l':	// live mode toggle
					gps_live_mode=!gps_live_mode;
					break;
					
		case '1':	Serial.println("1Hz NMEA rate");
					GPSSerialPort.println(PMTK_SET_NMEA_UPDATE_1HZ);
					break;
		
		case '2':	Serial.println("2Hz NMEA rate");
					GPSSerialPort.println(PMTK_SET_NMEA_UPDATE_2HZ);
					break;
		
		case '5':	Serial.println("5Hz NMEA rate");
					GPSSerialPort.println(PMTK_SET_NMEA_UPDATE_5HZ);
					break;
		
		case '0':	Serial.println("10Hz NMEA rate");
					GPSSerialPort.println(PMTK_SET_NMEA_UPDATE_10HZ);
					break;
		
		case 'g':	Serial.println("GPGGA only");
					GPSSerialPort.println(PMTK_SET_NMEA_OUTPUT_GGAONLY);
					break;		

		case 's':	Serial.println("GPGSA only");
					GPSSerialPort.println(PMTK_SET_NMEA_OUTPUT_GSAONLY);
					break;		
		
		case 'n':	Serial.println("All NMEA off");
					GPSSerialPort.println(PMTK_SET_NMEA_OUTPUT_OFF);
					break;
					
		case 'm':	Serial.println("GPGSA, GPGGA and GPRMC");
					GPSSerialPort.println(PMTK_SET_NMEA_OUTPUT_RMCGGAGSA);
					break;
		
		case 'x':	Serial.println("GPS current fix");
				    Serial.print(ss.gps_latitude,6);
					Serial.print(", ");
				    Serial.print(ss.gps_longitude,6);
					Serial.print(", ");
					Serial.print(ss.gps_altitude,1);
					Serial.println(" m");		
					break;
		
		case 'y':	Serial.println("GGA and RMC only");
					GPSSerialPort.println(PMTK_SET_NMEA_OUTPUT_RMCGGA);
					break;
		
		case 'a':	Serial.println("All NMEA on");
					GPSSerialPort.println(PMTK_SET_NMEA_OUTPUT_ALLDATA);
		
		case 'v':	Serial.println("Requesting version");
					GPSSerialPort.println(PMTK_Q_RELEASE);
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

#if 0
void onRmcUpdate(nmea::RmcData const rmc)
{
  Serial.print("RMC ");

  if      (rmc.source == nmea::RmcSource::GPS)     Serial.print("GPS");
  else if (rmc.source == nmea::RmcSource::GLONASS) Serial.print("GLONASS");
  else if (rmc.source == nmea::RmcSource::Galileo) Serial.print("Galileo");
  else if (rmc.source == nmea::RmcSource::GNSS)    Serial.print("GNSS");
  else if (rmc.source == nmea::RmcSource::BDS)     Serial.print("BDS");

  Serial.print(" ");
  Serial.print(rmc.time_utc.hour);
  Serial.print(":");
  Serial.print(rmc.time_utc.minute);
  Serial.print(":");
  Serial.print(rmc.time_utc.second);
  Serial.print(".");
  Serial.print(rmc.time_utc.microsecond);

  if (rmc.is_valid)
  {
    Serial.print(" : LON ");
    Serial.print(rmc.longitude);
    Serial.print(" ° | LAT ");
    Serial.print(rmc.latitude);
    Serial.print(" ° | VEL ");
    Serial.print(rmc.speed);
    Serial.print(" m/s | HEADING ");
    Serial.print(rmc.course);
    Serial.print(" °");
  }

  Serial.println();
}

void onGgaUpdate(nmea::GgaData const gga)
{
  Serial.print("GGA ");

  if      (gga.source == nmea::GgaSource::GPS)     Serial.print("GPS");
  else if (gga.source == nmea::GgaSource::GLONASS) Serial.print("GLONASS");
  else if (gga.source == nmea::GgaSource::Galileo) Serial.print("Galileo");
  else if (gga.source == nmea::GgaSource::GNSS)    Serial.print("GNSS");
  else if (gga.source == nmea::GgaSource::BDS)     Serial.print("BDS");

  Serial.print(" ");
  Serial.print(gga.time_utc.hour);
  Serial.print(":");
  Serial.print(gga.time_utc.minute);
  Serial.print(":");
  Serial.print(gga.time_utc.second);
  Serial.print(".");
  Serial.print(gga.time_utc.microsecond);

  if (gga.fix_quality != nmea::FixQuality::Invalid)
  {
    Serial.print(" : LON ");
    Serial.print(gga.longitude);
    Serial.print(" ° | LAT ");
    Serial.print(gga.latitude);
    Serial.print(" ° | Num Sat. ");
    Serial.print(gga.num_satellites);
    Serial.print(" | HDOP =  ");
    Serial.print(gga.hdop);
    Serial.print(" m | Altitude ");
    Serial.print(gga.altitude);
    Serial.print(" m | Geoidal Separation ");
    Serial.print(gga.geoidal_separation);
    Serial.print(" m");
  }

  Serial.println();
}
#endif
