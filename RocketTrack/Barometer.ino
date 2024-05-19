
#define DEBUG 1

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP085_U.h>

#include "GpsOnePPS.h"

int baro_enable=1;

bool baro_trigger=false;
int baro_gps_sync=0;

char baro_type[32]="Generic";

int baro_rate=100;
int baro_period=1000;	// 10Hz
int last_baro_time=0;

#define BME_ADDRESS	0x76

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
Adafruit_BMP085_Unified bmp=Adafruit_BMP085_Unified(10085);

float baro_temp=0.0f;
float baro_pressure=0.0f;
float baro_height=0.0f;
float baro_humidity=0.0f;

float max_baro_height=0.0f;

int SetupBarometer(void)
{
	if(strstr(baro_type,"None")!=NULL)
	{
		Serial.println("No barometer configured, disabling");
		baro_enable=0;	
	}
	else if(strstr(baro_type,"BME280")!=NULL)
	{
		if(!bme.begin(BME_ADDRESS))
		{
			Serial.println("BME280 barometer not found, disabling");
			baro_enable=0;
			
			return(1);
		}	
	}
	else if(strstr(baro_type,"BMP180")!=NULL)
	{
		/* Initialise the sensor */
		if(!bmp.begin())
		{
			/* There was a problem detecting the BMP085 ... check your connections */
			Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
			baro_enable=0;
		}
		else
		{
			/* Display some basic information on this sensor */
			displayBaroSensorDetails();
		}
	}
	else
	{
		Serial.println("No barometer configured, disabling");
		baro_enable=0;
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
			
	baro_temp=bme.readTemperature();
	baro_pressure=bme.readPressure()/100.0F;
	baro_height=bme.readAltitude(SEALEVELPRESSURE_HPA);
	baro_humidity=bme.readHumidity();

	if(max_baro_height<baro_height)
		max_baro_height=baro_height;

#if DEBUG>1
	Serial.print("Temperature = ");			Serial.print(baro_temp);		Serial.print(" *C\t");
	Serial.print("Pressure = ");			Serial.print(baro_pressure);	Serial.print(" hPa\t");
	Serial.print("Approx. Altitude = ");	Serial.print(baro_height);		Serial.print(" m\t");
	Serial.print("Humidity = ");			Serial.print(baro_humidity);	Serial.print(" %\t");
	Serial.println();
#endif
		
	if(logging_enable)
	{
	
	}
}

float ReadAltitude(void)
{
	if(strstr(baro_type,"BME280")!=NULL)
		return(bme.readAltitude(SEALEVELPRESSURE_HPA));

	if(strstr(baro_type,"BMP180")!=NULL)
	{
		float pressure;
		bmp.getPressure(&pressure);
		return(bmp.pressureToAltitude(SENSORS_PRESSURE_SEALEVELHPA,pressure/100.0F));
	}
}

float ReadPressure(void)
{
	if(strstr(baro_type,"BME280")!=NULL)
		return(bme.readPressure());

	if(strstr(baro_type,"BMP180")!=NULL)
	{
		float pressure;
		bmp.getPressure(&pressure);
		return(pressure);
	}
}

float ReadTemperature(void)
{
	if(strstr(baro_type,"BME280")!=NULL)
		return(bme.readTemperature());
	
	if(strstr(baro_type,"BMP180")!=NULL)
	{
		float temperature;
		bmp.getTemperature(&temperature);
		return(temperature);
	}
}

float ReadHumidity(void)
{
	if(strstr(baro_type,"BME280")!=NULL)
		return(bme.readHumidity());
	
	if(strstr(baro_type,"BMP180")!=NULL)
	{
		return(0.0F);
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
		case 'a':	Serial.print("Altitude: ");		Serial.print(ReadAltitude());		Serial.print(" m\r\n");
					break;
					
		case 'p':	Serial.print("Pressure: ");		Serial.print(ReadPressure());		Serial.print(" Pa\r\n");
					break;

		case 't':	Serial.print("Temperature: ");	Serial.print(ReadTemperature());	Serial.print(" *C\r\n");
					break;
		
		case 'h':	Serial.print("Humidity: ");		Serial.print(ReadHumidity());		Serial.print(" %\r\n");
					break;
		
		case 'r':	Serial.print("Altitude: ");		Serial.print(ReadAltitude());		Serial.print(" m\r\n");
					Serial.print("Pressure: ");		Serial.print(ReadPressure());		Serial.print(" Pa\r\n");
					Serial.print("Temperature: ");	Serial.print(ReadTemperature());	Serial.print(" *C\r\n");
					Serial.print("Humidity: ");		Serial.print(ReadHumidity());		Serial.print(" %\r\n");
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

void displayBaroSensorDetails(void)
{
	sensor_t sensor;
	bmp.getSensor(&sensor);
	
	Serial.println("------------------------------------");
	Serial.print  ("Sensor:       "); Serial.println(sensor.name);
	Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
	Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
	Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
	Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
	Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");  
	Serial.println("------------------------------------");
	Serial.println("");
	
	delay(500);
}

