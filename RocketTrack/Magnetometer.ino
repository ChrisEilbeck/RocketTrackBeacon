
#include "SensorState.h"

#include <Adafruit_LSM303DLH_Mag.h>
#include <Adafruit_Sensor.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303DLH_Mag_Unified mag=Adafruit_LSM303DLH_Mag_Unified(10003);

int mag_enable=1;

bool trigger_mag=false;

char mag_type[32]="Generic";
int mag_type_no=MAGNETOMETER_NONE;

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
		
		mag_type_no=MAGNETOMETER_LSM303DLHC;
		mag_enable=1;
	}
	else
	{
		Serial.println("Magnetometer mis-configured, disabling");
		mag_enable=0;
	}
	
	return(0);
}

void ReadMagnetometer(float *mag_x,float *mag_y,float *mag_z)
{
	sensors_event_t event;
	mag.getEvent(&event);

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

void PollMagnetometer(void)
{
	if(mag_enable)
	{
		if(sync_sampling)
		{
			if(trigger_mag)
			{
				ReadMagnetometer(&ss.mag_x,&ss.mag_y,&ss.mag_z);			
				trigger_mag=false;
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

void DisplayMagnetometerDetails(void)
{
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

