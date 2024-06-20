
#include "Accelerometer.h"
#include "Barometer.h"
#include "GPS.h"
#include "Gyro.h"
#include "HardwareAbstraction.h"
#include "Magnetometer.h"
#include "Packetisation.h"
#include "SensorState.h"


#include <Adafruit_GPS.h>

//#include <TinyGPS.h>
#include <TinyGPSPlus.h>
//#include <MicroNMEA.h>

//MicroNMEA library structures
//char gpsparserBuffer[100];
//MicroNMEA gpsparser(gpsparserBuffer,sizeof(gpsparserBuffer));

//TinyGPS gpsparser;

TinyGPSPlus gpsparser;

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
	
	

	// this also assumes we're using a u-Blox receiver, not always true



#if ARDUINO_TBEAM_USE_RADIO_SX1276
	// Pins for T-Beam v0.8 (3 push buttons) and up
	GPSSerialPort.begin(initial_baud,SERIAL_8N1,34,12);
#elif ARDUINO_TTGO_LoRa32_v21new
//	GPSSerialPort.begin(initial_baud,EspSoftwareSerial::SWSERIAL_8N1,GPS_RXD,GPS_TXD,false,95,11);

	Serial.println("Setting up for I2C GPS at address 0x10");

//	GPSSerialPort.begin(0x10);
	GPSSerialPort.begin(0x10);
	
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

#ifdef USE_FREERTOS
	// Create the queue which will have <QueueElementSize> number of elements, each of size `message_t` and pass the address to <QueueHandle>.
	QueueHandle=xQueueCreate(QueueElementSize,sizeof(message_t));

	// Check if the queue was successfully created
	if(QueueHandle==NULL)
	{
    	Serial.println("Queue could not be created. Halt.");
    	while(1)
    	{
    		delay(1000);  // Halt at this point as is not possible to continue
		}
	}
	
	xTaskCreatePinnedToCore(GPSReceiveTask,"GPS Input Task",20480,NULL,2,NULL,0);
	xTaskCreatePinnedToCore(GPSParserTask,"GPS Parser Task",2048,NULL,2,NULL,0);

//	xTaskCreate(GPSReceiveTask,"GPS Inpur Task",2048,NULL,2,NULL);
//	xTaskCreate(GPSParserTask,"GPS Parser Task",2048,NULL,2,NULL);
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
		
							break;
	
		default:			// assume this is just a generic GPS and leave the
							// messages it output as is.
							Serial.println("\tConfiguring as a standard GPS, no custom messages selected");
							break;
	}
}

#ifdef USE_FREERTOS
void GPSReceiveTask(void *pvParameters)
{
	char rxbyte;

	while(1)
	{
//		xSemaphoreTake(i2c_mutex,portMAX_DELAY);

//		int bytecount=0;
//		message_t message;
		
		while(GPSSerialPort.available())
		{
			rxbyte=GPSSerialPort.read();
			
			if(gps_live_mode)
				Serial.write(rxbyte);

//			if(gpsparser.process(rxbyte))
//			{
//				Serial.println(gpsparser.getSentence());
//			}

			gpsparser.encode(rxbyte);
					
			if(rxbyte=='\n')
			{
				Serial.print(F("Location: ")); 
				if(gpsparser.location.isValid())
				{
					Serial.print(gpsparser.location.lat(), 6);
					Serial.print(F(","));
					Serial.print(gpsparser.location.lng(), 6);
				}
				else
				{
					Serial.print(F("INVALID"));
				}

				Serial.print(F("		Date/Time: "));
				if(gpsparser.date.isValid())
				{
					Serial.print(gpsparser.date.month());
					Serial.print(F("/"));
					Serial.print(gpsparser.date.day());
					Serial.print(F("/"));
					Serial.print(gpsparser.date.year());
				}
				else
				{
					Serial.print(F("INVALID"));
				}

				Serial.print(F(" "));
				if(gpsparser.time.isValid())
				{
					if(gpsparser.time.hour() < 10) Serial.print(F("0"));
					Serial.print(gpsparser.time.hour());
					Serial.print(F(":"));
					if(gpsparser.time.minute() < 10) Serial.print(F("0"));
					Serial.print(gpsparser.time.minute());
					Serial.print(F(":"));
					if(gpsparser.time.second() < 10) Serial.print(F("0"));
					Serial.print(gpsparser.time.second());
					Serial.print(F("."));
					if(gpsparser.time.centisecond() < 10) Serial.print(F("0"));
					Serial.print(gpsparser.time.centisecond());
				}
				else
				{
					Serial.print(F("INVALID"));
				}
				
				Serial.println("");
			}

			// send it via a a message queue to the gps parser	
//			message.rxbytes[bytecount++]=rxbyte;
			
//			if(rxbyte=='\n')
//			{
//				message.count=bytecount+1;
//				message.rxbytes[bytecount]=0;
							
//				Serial.print("Sending ");
//				Serial.print(message.count);
//				Serial.print(" bytes: ");
//				Serial.println((char *)message.rxbytes);

//				break;

#if 0							
				// Check if the queue exists AND if there is any free space in the queue
				if(QueueHandle!=NULL&&uxQueueSpacesAvailable(QueueHandle)>0)
				{
					// The line needs to be passed as pointer to void.
					// The last parameter states how many milliseconds should wait (keep trying to send) if is not possible to send right away.
					// When the wait parameter is 0 it will not wait and if the send is not possible the function will return errQUEUE_FULL
					int ret=xQueueSend(QueueHandle,(void *)&message,0);
					if(ret==pdTRUE)
					{
						// The message was successfully sent.
					}
					else if(ret==errQUEUE_FULL)
					{
						// Since we are checking uxQueueSpacesAvailable this should not occur, however if more than one task should
						//   write into the same queue it can fill-up between the test and actual send attempt
						Serial.println("The `TaskReadFromSerial` was unable to send data into the Queue");
					}  // Queue send check
				}  // Queue sanity check
#endif
//			}
		}

//		xSemaphoreGive(i2c_mutex);
		delay(1);
	}
}

void GPSParserTask(void *pvParameters)
{
	
	while(1)
	{
		// One approach would be to poll the function (uxQueueMessagesWaiting(QueueHandle) and call delay if nothing is waiting.
		// The other approach is to use infinite time to wait defined by constant `portMAX_DELAY`:
		if(QueueHandle!=NULL)	// Sanity check just to make sure the queue actually exists
		{
			message_t message;
			int ret=xQueueReceive(QueueHandle,&message,portMAX_DELAY);
			if(ret==pdPASS)
      		{
      			// The message was successfully received - send it back to Serial port and "Echo: "
				Serial.print(message.rxbytes);
			}
			else if(ret==pdFALSE)
			{
				Serial.println("The `TaskWriteToSerial` was unable to receive data from the Queue");
			}
		}  // Sanity check
		
		delay(1);
	}
}
#else
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
#endif




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
					Serial.printf("Lat = %.6f, Lon = %.6f, ",ss.gps_latitude,ss.gps_longitude);
					Serial.printf("height = %.1f\r\n",ss.gps_altitude);
					break;
		
		case 'f':	// fix status
					if(ss.gps_fix==0x00)		Serial.println("No Fix");
					else if(ss.gps_fix==0x02)	Serial.println("2D Fix");
					else if(ss.gps_fix==0x03)	Serial.println("3D Fix");
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

