
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include <string.h>

int acc_enable=1;

char acc_type[32]="None";
int acc_type_no=ACCELEROMETER_NONE;

int acc_rate=200;

Adafruit_MPU6050 mpu;

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
		
		Serial.print("MPU6050 accelerometer configured\r\n");
		acc_type_no=ACCELEROMETER_MPU6050;
		acc_enable=1;
	}
	else if(strstr(acc_type,"LSM303DLHC")!=NULL)
	{
	
	
	}
	else if(strstr(acc_type,"ADXL345")!=NULL)
	{
		
		
		
		
		
		Serial.print("ADXL345 accelerometer configured\r\n");
		acc_type_no=ACCELEROMETER_ADXL345;
		acc_enable=1;
	}
	else
	{
		Serial.println("Accelerometer mis-configured, disabling");
		acc_enable=0;
	}
	
	return(0);
}

void PollAccelerometer(void)
{
	if(acc_enable)
	{
		switch(acc_type_no)
		{
			case ACCELEROMETER_NONE:	Serial.print("Accelerometer misconfigured, disabling\r\n");
										acc_enable=0;
										break;
			
			case ACCELEROMETER_MPU6050:	sensors_event_t a,g,temp;
										mpu.getEvent(&a,&g,&temp);
										
										Serial.print("Acceleration X: ");	Serial.print(a.acceleration.x);	Serial.print(", Y: ");	Serial.print(a.acceleration.y);	Serial.print(", Z: ");	Serial.print(a.acceleration.z);	Serial.print(" m/s^2\t");
#if 0
										Serial.print("Rotation X: ");		Serial.print(g.gyro.x);			Serial.print(", Y: ");	Serial.print(g.gyro.y);			Serial.print(", Z: ");	Serial.print(g.gyro.z);			Serial.print(" rad/s\t");
										Serial.print("Temperature: ");		Serial.print(temp.temperature);	Serial.print(" degC\t");
#endif
										
										break;
			
			case ACCELEROMETER_MPU9250:	Serial.print("Accelerometer type not supported yet, disabling\r\n");
										acc_enable=0;
										break;
			
			case ACCELEROMETER_ADXL345:	Serial.print("Accelerometer type not supported yet, disabling\r\n");
										acc_enable=0;
										break;
			
			default:					Serial.print("Accelerometer misconfigured, disabling\r\n");
										acc_enable=0;
										break;
		}
	}
}

int AccelerometerCommandHandler(uint8_t *cmd,uint16_t cmdptr)
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
		case 'r':	Serial.print("Read accelerometer:\t");
					sensors_event_t a,g,temp;
					mpu.getEvent(&a,&g,&temp);
										
					Serial.print("X: ");	Serial.print(a.acceleration.x);	
					Serial.print(", Y: ");	Serial.print(a.acceleration.y);	
					Serial.print(", Z: ");	Serial.print(a.acceleration.z);	Serial.print(" m/s^2\r\n");
					
					break;
					
		case '?':	Serial.print("Accelerometer Test Harness\r\n================\r\n\n");
					Serial.print("r\t-\tRead sensor\r\n");
					Serial.print("?\t-\tShow this menu\r\n");
					break;
		
		default:	retval=0;
					break;
	}
	
	return(retval);
}

