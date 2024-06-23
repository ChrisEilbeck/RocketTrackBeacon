
#define DEBUG	1

#include "SensorState.h"

#include <Adafruit_LSM303DLH_Mag.h>
#include <Adafruit_Sensor.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303DLH_Mag_Unified mag=Adafruit_LSM303DLH_Mag_Unified(10003);

bool mag_enable=false;

bool mag_trigger=false;

char mag_type[32]="Generic";
int mag_type_num=MAGNETOMETER_NONE;

int mag_period=100;
int last_mag_time=0;
int mag_rate=10;

int SetupMagnetometer(void)
{
	if(strstr(mag_type,"None")!=NULL)
	{
		Serial.println("No magnetometer configured, disabling");
		mag_enable=0;	
	}
	else if(strstr(mag_type,"LSM303DLHC")!=NULL)
	{
		// Enable auto-gain
		mag.enableAutoRange(true);

		// Initialise the sensor
		if(!mag.begin())
		{
			Serial.println("Ooops, no LSM303DLHC detected ... Check your wiring!");
			mag_enable=0;
			return(1);
		}

		Serial.println("LSM303DLCH magnetomere configured");

		DisplayMagnetometerDetails();
		
		mag_type_num=MAGNETOMETER_LSM303DLHC;
		mag_enable=1;
	}
	else
	{
		Serial.println("Magnetometer mis-configured, disabling");
		mag_enable=0;
	}

	mag_period=1000/mag_rate;

#ifdef USE_FREERTOS	
	xTaskCreatePinnedToCore(PollMagnetometer,"Magnetometer Task",2048,NULL,2,NULL,0);
//	xTaskCreate(PollMagnetometer,"Magnetometer Task",2048,NULL,2,NULL);
#endif
		
	return(0);
}

void ReadMagnetometer(float *mag_x,float *mag_y,float *mag_z)
{
//	Serial.println("\t\t\t\tSampling the Magnetometer");
	Serial.println("\t\t\tM ...");

	xSemaphoreTake(i2c_mutex,portMAX_DELAY);

	sensors_event_t event;
	mag.getEvent(&event);

	xSemaphoreGive(i2c_mutex);
	
	*mag_x=event.magnetic.x;
	*mag_y=event.magnetic.y;
	*mag_z=event.magnetic.z;

#if DEBUG>1
	Serial.print("X: ");
	Serial.print(event.magnetic.x);
	Serial.print("  ");
	Serial.print("Y: ");
	Serial.print(event.magnetic.y);
	Serial.print("  ");
	Serial.print("Z: ");
	Serial.print(event.magnetic.z);
	Serial.print("  ");
	Serial.println("uT");
#endif
}

#ifdef USE_FREERTOS
void PollMagnetometer(void *pvParameters)
{
	delay(1000);
	
	while(1)
	{
		if(mag_enable)
		{
			if(sync_sampling)
			{
				if(mag_trigger)
				{
					ReadMagnetometer(&ss.mag_x,&ss.mag_y,&ss.mag_z);
					mag_trigger=false;
				}
				else
					delay(1);
			}
			else
			{
				ReadMagnetometer(&ss.mag_x,&ss.mag_y,&ss.mag_z);
				delay(mag_period);
			}
		}
		else
		{
			ss.mag_x=0.0f;	ss.mag_y=0.0f;	ss.mag_z=0.0f;
			delay(mag_period);
		}		
	}
}
#else
void PollMagnetometer(void)
{
	if(mag_enable)
	{
		if(sync_sampling)
		{
			if(mag_trigger)
			{
				ReadMagnetometer(&ss.mag_x,&ss.mag_y,&ss.mag_z);			
				mag_trigger=false;
			}
		}
		else
		{
			if(millis()>(last_mag_time+mag_period))
			{
				ReadMagnetometer(&ss.mag_x,&ss.mag_y,&ss.mag_z);
				last_mag_time=millis();
			}
		}
	}
	else
	{
		ss.mag_x=0.0f;	ss.mag_y=0.0f;	ss.mag_z=0.0f;
	}
}
#endif

void DisplayMagnetometerDetails(void)
{
#if (DEBUG>0)
	sensor_t sensor;
	mag.getSensor(&sensor);
	
	Serial.println("------------------------------------");
	Serial.print("Sensor:       ");
	Serial.println(sensor.name);
	Serial.print("Driver Ver:   ");
	Serial.println(sensor.version);
	Serial.print("Unique ID:    ");
	Serial.println(sensor.sensor_id);
	Serial.print("Max Value:    ");
	Serial.print(sensor.max_value);
	Serial.println(" uT");
	Serial.print("Min Value:    ");
	Serial.print(sensor.min_value);
	Serial.println(" uT");
	Serial.print("Resolution:   ");
	Serial.print(sensor.resolution);
	Serial.println(" uT");
	Serial.println("------------------------------------");
	Serial.println("");
#endif
}

int MagnetometerCommandHandler(uint8_t *cmd,uint16_t cmdptr)
{
	// ignore a single key stroke
	if(cmdptr<=2)	return(0);
	
#if (DEBUG>0)
	Serial.println((char *)cmd);
#endif
	
	int retval=1;
	
	switch(cmd[1]|0x20)
	{
		case 'r':	{
						float mag_x;
						float mag_y;
						float mag_z;
						
						ReadMagnetometer(&mag_x,&mag_y,&mag_z);
						Serial.print("Magnetic Field X: ");	Serial.print(mag_x);	Serial.print(", Y: ");	Serial.print(mag_y);	Serial.print(", Z: ");	Serial.print(mag_z);	Serial.println(" uT");
					}
					
					break;
		
		case '?':	Serial.print("Magnetometer Test Harness\r\n================\r\n\n");
					Serial.print("r\t-\tRead sensor\r\n");
					Serial.print("?\t-\tShow this menu\r\n");
					break;
		
		default:	retval=0;
					break;
	}
	
	return(retval);
}

