
#include "SensorState.h"

#include <Adafruit_LSM303_Accel.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include <string.h>

int acc_enable=1;
char acc_type[32]="None";
int acc_type_num=ACCELEROMETER_NONE;
int last_accel_time=0;
int accel_period=100;
int acc_rate=10;

bool trigger_accel=false;

Adafruit_MPU6050 mpu;
Adafruit_LSM303_Accel_Unified accel=Adafruit_LSM303_Accel_Unified(10001);

void DisplayAccelerometerDetails(void)
{
	sensor_t sensor;
	accel.getSensor(&sensor);

	Serial.println("------------------------------------");
	Serial.print("Sensor:       ");
	Serial.println(sensor.name);
	Serial.print("Driver Ver:   ");
	Serial.println(sensor.version);
	Serial.print("Unique ID:    ");
	Serial.println(sensor.sensor_id);
	Serial.print("Max Value:    ");
	Serial.print(sensor.max_value);
	Serial.println(" m/s^2");
	Serial.print("Min Value:    ");
	Serial.print(sensor.min_value);
	Serial.println(" m/s^2");
	Serial.print("Resolution:   ");
	Serial.print(sensor.resolution);
	Serial.println(" m/s^2");
	Serial.println("------------------------------------");
	Serial.println("");
}

int SetupAccelerometer(void)
{
	if(strstr(acc_type,"None")!=NULL)
	{
		Serial.println("No accelerometer configured, disabling");
		acc_enable=0;	
	}
	else if(strstr(acc_type,"MPU6050")!=NULL)
	{
		if(!mpu.begin())
		{
			Serial.println("MPU6050 not detected, disabling");
			acc_enable=0;
			return(1);
		}
		
		mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
		mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
		
		Serial.println("MPU6050 accelerometer configured");
		acc_type_num=ACCELEROMETER_MPU6050;
		acc_enable=1;
	}
	else if(strstr(acc_type,"LSM303DLHC")!=NULL)
	{
		if(!accel.begin())
		{
			Serial.println("LSM303DLHC not detected, disabling");
			acc_enable=0;
			return(1);
		}
		
		accel.setRange(LSM303_RANGE_16G);
		accel.setMode(LSM303_MODE_NORMAL);
		
		Serial.println("LSM303DLHC accelerometer configured");
	
		DisplayAccelerometerDetails();
	
		acc_type_num=ACCELEROMETER_LSM303DLHC;
		acc_enable=1;
	}
	else if(strstr(acc_type,"ADXL345")!=NULL)
	{
		Serial.print("ADXL345 accelerometer configured\r\n");
		acc_type_num=ACCELEROMETER_ADXL345;
		acc_enable=1;
	}
	else
	{
		Serial.println("Accelerometer mis-configured, disabling");
		acc_enable=0;
	}

#ifdef USE_FREERTOS	
	xTaskCreate(PollAccelerometer,"Accel Task",2048,NULL,2,NULL);
#endif
	
	return(0);
}

void ReadAccelerometer(float *accel_x,float *accel_y,float *accel_z)
{
	sensors_event_t a;
	sensors_event_t g;
	sensors_event_t temp;
			
	switch(acc_type_num)
	{
		case ACCELEROMETER_NONE:		Serial.print("Accelerometer misconfigured, disabling\r\n");
										acc_enable=0;
										break;
		
		case ACCELEROMETER_MPU6050:		mpu.getEvent(&a,&g,&temp);
										*accel_x=a.acceleration.x;
										*accel_y=a.acceleration.y;
										*accel_z=a.acceleration.z;
										break;
		
		case ACCELEROMETER_LSM303DLHC:	accel.getEvent(&a);
										*accel_x=a.acceleration.x;
										*accel_y=a.acceleration.y;
										*accel_z=a.acceleration.z;
										break;
		
		case ACCELEROMETER_MPU9250:		Serial.print("Accelerometer type not supported yet, disabling\r\n");
										acc_enable=0;
										break;
		
		case ACCELEROMETER_ADXL345:		Serial.print("Accelerometer type not supported yet, disabling\r\n");
										acc_enable=0;
										break;
		
		default:						Serial.print("Accelerometer misconfigured, disabling\r\n");
										acc_enable=0;
										break;
	}
}

#ifdef USE_FREERTOS
void PollAccelerometer(void *pvParameters)
{
	while(1)
	{
		if(acc_enable)
		{
			if(sync_sampling)
			{
				if(trigger_accel)
				{
					ReadAccelerometer(&ss.accel_x,&ss.accel_y,&ss.accel_z);
					trigger_accel=false;
				}
			}
			else
			{
				ReadAccelerometer(&ss.accel_x,&ss.accel_y,&ss.accel_z);
			}
			
			delay(accel_period);
		}
		else
		{
			ss.accel_x=0.0f;	ss.accel_y=0.0f;	ss.accel_z=0.0f;
			delay(1000);
		}
	}
}
#else
void PollAccelerometer(void)
{
	if(acc_enable)
	{
		if(sync_sampling)
		{
			if(trigger_accel)
			{
				ReadAccelerometer(&ss.accel_x,&ss.accel_y,&ss.accel_z);
				trigger_accel=false;
			}
		}
		else
		{	
			if(millis()>(last_accel_time+accel_period))
			{
				ReadAccelerometer(&ss.accel_x,&ss.accel_y,&ss.accel_z);
				last_accel_time=millis();
			}
		}
	}
	else
	{
		ss.accel_x=0.0f;	ss.accel_y=0.0f;	ss.accel_z=0.0f;
	}
}
#endif

int AccelerometerCommandHandler(uint8_t *cmd,uint16_t cmdptr)
{
	// ignore a single key stroke
	if(cmdptr<=2)	return(0);
	
#if (DEBUG>0)
	Serial.println((char *)cmd);
#endif
	
	int retval=1;
	
	switch(cmd[1]|0x20)
	{
		case 'd':	Serial.println("Disabling the Accelerometer");
					acc_enable=false;
					break;
					
		case 'e':	Serial.println("Enabling the Accelerometer");
					acc_enable=true;
					break;
		
		case 'r':	{
						float accel_x;
						float accel_y;
						float accel_z;
						
						ReadAccelerometer(&accel_x,&accel_y,&accel_z);
						Serial.print("Accel X: ");	Serial.print(accel_x);	Serial.print(", Y: ");	Serial.print(accel_y);	Serial.print(", Z: ");	Serial.print(accel_z);	Serial.println(" m/s^2");
					}
					
					break;
		
		case 't':	Serial.print("Accel X: ");	Serial.print(ss.accel_x);	Serial.print(", Y: ");	Serial.print(ss.accel_y);	Serial.print(", Z: ");	Serial.print(ss.accel_z);	Serial.println(" m/s^2");
					break;
		
		case '?':	Serial.print("Accelerometer Test Harness\r\n================\r\n\n");
					Serial.print("d\t-\tDisable the Accelerometer");
					Serial.print("e\t-\tEnable the Accelerometer");
					Serial.print("r\t-\tRead sensor\r\n");
					Serial.print("t\t-\tShow last reading\r\n");
					Serial.print("?\t-\tShow this menu\r\n");
					break;
		
		default:	retval=0;
					break;
	}
	
	return(retval);
}

