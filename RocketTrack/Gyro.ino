
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>



int gyro_enable=1;

char gyro_type[32]="None";
int gyro_type_no=0;

int gyro_rate=200;

extern Adafruit_MPU6050 mpu;

int SetupGyro(void)
{
	if(strstr(gyro_type,"None")!=NULL)
	{
		Serial.println("No gyro configured, disabling");
		gyro_enable=0;	
	}
	else if(strstr(gyro_type,"MPU6050")!=NULL)
	{
		if(!mpu.begin())
		{
			Serial.println("MPU6050 not detected, disabling");
			gyro_enable=0;
			return(1);
		}

		mpu.setGyroRange(MPU6050_RANGE_500_DEG);
		mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
		
		Serial.print("MPU6050 gyro configured\r\n");
		gyro_type_no=GYRO_MPU6050;
		gyro_enable=1;
	}
	else if(strstr(gyro_type,"L3GD20")!=NULL)
	{
	
	
	
		gyro_enable=0;
	}
	else
	{
		Serial.println("No gyro configured, disabling");
		gyro_enable=0;
	}
	
	return(0);
}

void PollGyro(void)
{
	if(gyro_enable)
	{
		sensors_event_t a,g,temp;
		mpu.getEvent(&a,&g,&temp);
		Serial.print("Rotation X: ");		Serial.print(g.gyro.x);			Serial.print(", Y: ");	Serial.print(g.gyro.y);			Serial.print(", Z: ");	Serial.print(g.gyro.z);			Serial.print(" rad/s\t");
	}
}

int GyroCommandHandler(uint8_t *cmd,uint16_t cmdptr)
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
		case 'r':	Serial.print("Read gyro:\t");
					sensors_event_t a,g,temp;
					mpu.getEvent(&a,&g,&temp);
										
					Serial.print("X: ");	Serial.print(g.acceleration.x);	
					Serial.print(", Y: ");	Serial.print(g.acceleration.y);	
					Serial.print(", Z: ");	Serial.print(g.acceleration.z);	Serial.print(" degs/s\r\n");
					
					break;
					
		case '?':	Serial.print("Gyro Test Harness\r\n================\r\n\n");
					Serial.print("r\t-\tRead sensor\r\n");
					Serial.print("?\t-\tShow this menu\r\n");
					break;
		
		default:	retval=0;
					break;
	}
	
	return(retval);
}

