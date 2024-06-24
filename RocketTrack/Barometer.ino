
#define DEBUG 1

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP085_U.h>

#include "GPS.h"
#include "GpsOnePPS.h"
#include "SensorState.h"

bool baro_enable=false;

bool baro_trigger=false;
bool baro_gps_sync=false;

char baro_type[32]="Generic";
int baro_type_num=BAROMETER_NONE;

int baro_rate=10;
int baro_period=100;
int last_baro_time=0;

#define BME_ADDRESS	0x76

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
Adafruit_BMP085_Unified bmp=Adafruit_BMP085_Unified(10004);

int SetupBarometer(void)
{
	if(strstr(baro_type,"None")!=NULL)
	{
		Serial.println("No barometer configured, disabling");
		baro_enable=false;	
	}
	else if(strstr(baro_type,"BME280")!=NULL)
	{
		if(!bme.begin(BME_ADDRESS))
		{
			Serial.println("BME280 barometer not found, disabling");
			baro_enable=false;
			return(1);
		}	
	}
	else if(		(strstr(baro_type,"BMP180")!=NULL)
				||	(strstr(baro_type,"BMP085")!=NULL)	)
	{
		/* Initialise the sensor */
		if(!bmp.begin(BMP085_MODE_STANDARD))
		{
			Serial.print("Ooops, no BMP085/BMP180 detected, disabling");
			baro_enable=false;
			return(1);
		}
		
		Serial.println("BMP085/BMP180 barometer configured");

		DisplayBarometerDetails();
		
		baro_type_num=BAROMETER_BMP085;
		baro_enable=true;
	}
	else
	{
		Serial.println("No barometer configured, disabling");
		baro_enable=false;
	}
	
	baro_period=1000/baro_rate;
	
	Serial.printf("Baro period = %d\r\n",baro_period);
	
	Serial.println("Barometer configured\n");

#ifdef USE_FREERTOS	
	xTaskCreatePinnedToCore(PollBarometer,"Barometer Task",2048,NULL,2,NULL,0);
//	xTaskCreate(PollBarometer,"Barometer Task",2048,NULL,2,NULL);
#endif
	
	return(0);
}

#ifdef USE_FREERTOS
void PollBarometer(void *pvParameters)
{
	// sample the barometer for a second, average and use that as the launch altitude

	float avg_alt=0.0f;
	for(int cnt=0;cnt<100;cnt++)
	{
		ReadBarometer();
		avg_alt+=ss.baro_altitude;	
		delay(10);
	}
	
	ss.baro_launch_altitude=avg_alt/100.0f;
	
	Serial.print("Launch altitude stored: ");
	Serial.print(ss.baro_launch_altitude);
	Serial.println(" m");
	
	while(1)
	{
		if(baro_enable)
		{
#if 1
			if(sync_sampling)
			{
				if(baro_trigger)
				{
					ReadBarometer();
					baro_trigger=false;
					accel_trigger=true;
				}
				else
					delay(1);
			}
			else
#endif
			{
				ReadBarometer();		
				accel_trigger=true;
				delay(baro_period);
			}
		}
		else
		{
			ss.baro_altitude=0.0f;	ss.baro_pressure=0.0f;	ss.baro_temperature=0.0f;	ss.baro_humidity=0.0f;
			delay(baro_period);
		}		
	}
}
#else
void PollBarometer(void)
{
	if(baro_enable)
	{
		if(millis()>(last_baro_time+baro_period))
		{
			ReadBarometer();
			last_baro_time=millis();
		}
	}
	else
	{
		ss.baro_altitude=0.0f;	ss.baro_pressure=0.0f;	ss.baro_temperature=0.0f;	ss.baro_humidity=0.0f;
	}
}
#endif

void ReadBarometer(void)
{
//	Serial.println("\n\tSampling the Barometer");
	Serial.println("B-");

	xSemaphoreTake(i2c_mutex,portMAX_DELAY);

	ss.baro_altitude=ReadAltitude();
	ss.baro_pressure=ReadPressure();
	ss.baro_temperature=ReadTemperature();
	ss.baro_humidity=ReadHumidity();

	xSemaphoreGive(i2c_mutex);

	if(ss.baro_max_altitude<ss.baro_altitude)
		ss.baro_max_altitude=ss.baro_altitude;

#if DEBUG>1
	Serial.print("Approx. Altitude = ");	Serial.print(ss.baro_altitude);	Serial.print(" m\t");
	Serial.print("Pressure = ");			Serial.print(ss.baro_pressure);	Serial.print(" hPa\t");
	Serial.print("Temperature = ");			Serial.print(ss.baro_temp);		Serial.print(" *C\t");
	Serial.print("Humidity = ");			Serial.print(ss.baro_humidity);	Serial.print(" %\t");
	Serial.println();
#endif
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
	
	return(0.0F);
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
	
	return(0.0F);
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
	
	return(0.0F);
}

float ReadHumidity(void)
{
	if(strstr(baro_type,"BME280")!=NULL)
		return(bme.readHumidity());
	
	return(0.0F);
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
		case 'a':	ReadBarometer();
					Serial.print("Altitude: ");		Serial.print(ss.baro_altitude);		Serial.print(" m\r\n");
					break;
		
		case 'p':	ReadBarometer();
					Serial.print("Pressure: ");		Serial.print(ss.baro_pressure);		Serial.print(" Pa\r\n");
					break;
		
		case 't':	ReadBarometer();
					Serial.print("Temperature: ");	Serial.print(ss.baro_temperature);	Serial.print(" *C\r\n");
					break;
		
		case 'h':	ReadBarometer();
					Serial.print("Humidity: ");		Serial.print(ss.baro_humidity);		Serial.print(" %\r\n");
					break;
		
		case 'r':	ReadBarometer();
					Serial.print("Altitude: ");		Serial.print(ss.baro_altitude);		Serial.print(" m\r\n");
					Serial.print("Pressure: ");		Serial.print(ss.baro_pressure);		Serial.print(" Pa\r\n");
					Serial.print("Temperature: ");	Serial.print(ss.baro_temperature);	Serial.print(" *C\r\n");
					Serial.print("Humidity: ");		Serial.print(ss.baro_humidity);		Serial.print(" %\r\n");		
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

void DisplayBarometerDetails(void)
{
#if (DEBUG>0)
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
#endif
}

