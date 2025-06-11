
#define DEBUG 1

#include <Wire.h>
#include <Adafruit_Sensor.h>

#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>

#include "GpsOnePPS.h"
#include "Logging.h"

#include "Packetisation.h"

int baro_enable=1;

bool baro_trigger=false;
int baro_gps_sync=0;

int baro_rate=100;
int baro_period=1000;	// 10Hz
int last_baro_time=0;

#define BME_ADDRESS	0x76
#define BMP_ADDRESS	0x76

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
Adafruit_BMP280 bmp;

int baro_sensor_type=NO_BARO;

float baro_temp=0.0f;
float baro_pressure=0.0f;
float baro_height=0.0f;
float baro_humidity=0.0f;

float max_baro_height=0.0f;

int SetupBarometer(void)
{
	bool fail=true;
	
	if(fail&&bme.begin(BME_ADDRESS))
	{
		Serial.println("BME280 barometer found");
		baro_sensor_type=BME280;
		fail=false;
	}	
	
	if(fail&&!bmp.begin(BMP_ADDRESS))
	{
		Serial.println("BMP280 barometer found");
		baro_sensor_type=BMP280;
		fail=false;
	}	
	
	if(fail)
	{
		Serial.println("No barometer sensors found");
		return(1);
	}
	
	if(baro_rate!=0)
		baro_period=1000/baro_rate;
	
	Serial.printf("Baro period = %d\r\n",baro_period);
	
	Serial.print("Barometer configured\r\n");

	last_baro_time=millis_1pps();
	
	return(0);
}

void PollBarometer(void)
{
	if(baro_enable)
	{
		if(baro_gps_sync)
		{
			if(baro_trigger)
			{
				SampleBarometer();
				baro_trigger=false;
			}
		}
		else
		{
			if(millis_1pps()>(last_baro_time+baro_period))
			{
				SampleBarometer();
				last_baro_time=millis_1pps();
			}
		}
	}
}

void SampleBarometer(void)
{
#if DEBUG>2
	Serial.println(millis_1pps());
#endif
			
	switch(baro_sensor_type)
	{
		case BME280:	baro_temp=bme.readTemperature();
						baro_pressure=bme.readPressure()/100.0F;
						baro_height=bme.readAltitude(SEALEVELPRESSURE_HPA);
						baro_humidity=bme.readHumidity();
						
						
						break;

		case BMP280:	baro_temp=bmp.readTemperature();
						baro_pressure=bmp.readPressure()/100.0F;
						baro_height=bmp.readAltitude(SEALEVELPRESSURE_HPA);
						
						lastfix.height=baro_height;
						
						break;

		default:		Serial.println("Should never hit this point, trying to read a non-existent barometer!");
						baro_enable=false;
						break;
	}

	lastfix.height=baro_height;
	
	if(max_baro_height<baro_height)
		max_baro_height=baro_height;

#if DEBUG>1
	Serial.print("Temperature = ");			Serial.print(baro_temp);		Serial.print(" *C\t");
	Serial.print("Pressure = ");			Serial.print(baro_pressure);	Serial.print(" hPa\t");
	Serial.print("Approx. Altitude = ");	Serial.print(baro_height);		Serial.print(" m\t");
	
	if(baro_sensor_type==BME280)
		Serial.print("Humidity = ");			Serial.print(baro_humidity);	Serial.print(" %\t");
	
	Serial.println();
#endif
		
	if(logging_enable)
	{
	
	}
}

int BarometerCommandHandler(uint8_t *cmd,uint16_t cmdptr)
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
		case 'a':	SampleBarometer();
					Serial.print("Altitude: ");		Serial.print(baro_height);		Serial.print(" m\r\n");
					break;
					
		case 'p':	SampleBarometer();
					Serial.print("Pressure: ");		Serial.print(baro_pressure);	Serial.print(" hPa\r\n");
					break;

		case 't':	SampleBarometer();
					Serial.print("Temperature: ");	Serial.print(baro_temp);		Serial.print(" *C\r\n");
					break;
		
		case 'h':	if(baro_sensor_type==BME280)
					{
						SampleBarometer();
						Serial.print("Humidity = ");	Serial.print(baro_humidity);	Serial.print(" %\r\n");
					}
					
					break;
		
		case 'r':	SampleBarometer();
					
					Serial.print("Altitude: ");		Serial.print(baro_height);		Serial.print(" m\r\n");					
					Serial.print("Pressure: ");		Serial.print(baro_pressure);	Serial.print(" hPa\r\n");
					Serial.print("Temperature: ");	Serial.print(baro_temp);		Serial.print(" *C\r\n");

					if(baro_sensor_type==BME280)
					{
						SampleBarometer();
						Serial.print("Humidity = ");	Serial.print(baro_humidity);	Serial.print(" %\r\n");
					}
					
					break;
		
		case '?':	Serial.print("Barometer Test Harness\r\n================\r\n\n");
					Serial.print("a\t-\tRead altitude\r\n");
					Serial.print("p\t-\tRead pressure\r\n");
					Serial.print("t\t-\tRead temperature\r\n");
					Serial.print("h\t-\tRead humidity\r\n");
					Serial.print("r\t-\tRead all sensors\r\n");
					Serial.print("?\t-\tShow this menu\r\n");
					break;
		
		default:	retval=0;
					break;
	}
	
	return(retval);
}

