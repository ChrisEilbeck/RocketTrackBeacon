
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

int gyro_enable=1;

char gyro_type[32]="None";
int gyro_type_no=0;
int last_gyro_time=0;
int gyro_period=100;
int gyro_rate=10;

extern Adafruit_MPU6050 mpu;
Adafruit_L3GD20_Unified gyro=Adafruit_L3GD20_Unified(10002);

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
		
		Serial.println("MPU6050 gyro configured\r\n");
		
		gyro_type_no=GYRO_MPU6050;
		gyro_enable=1;
	}
	else if(strstr(gyro_type,"L3GD20")!=NULL)
	{
		gyro.enableAutoRange(true);
	
		if(!gyro.begin())
		{
			Serial.println("L3GD20 not detected, disabling");		
			gyro_enable=0;
			return(1);
		}	
	
		Serial.println("L3GD20 gyro configured");
		
		DisplayGyroDetails();
		
		gyro_type_no=GYRO_L3GD20;
		gyro_enable=1;
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
		if(millis()>(last_gyro_time+gyro_period))
		{
			SampleGyro();
			last_gyro_time=millis();
		}
	}
}

void SampleGyro(void)
{
	sensors_event_t g;
	gyro.getEvent(&g);
	
	Serial.print("Rotation X: ");
	Serial.print(g.gyro.x);
	Serial.print(", Y: ");
	Serial.print(g.gyro.y);
	Serial.print(", Z: ");
	Serial.print(g.gyro.z);
	Serial.println(" rad/s\t");
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

void DisplayGyroDetails(void)
{
	sensor_t sensor;
	gyro.getSensor(&sensor);

	Serial.println("------------------------------------");
	Serial.print  ("Sensor:       "); Serial.println(sensor.name);
	Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
	Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
	Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" rad/s");
	Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" rad/s");
	Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" rad/s");  
	Serial.println("------------------------------------");
	Serial.println("");
}

