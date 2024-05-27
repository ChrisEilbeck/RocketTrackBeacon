
#include <Adafruit_LSM303_Accel.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include <string.h>

int acc_enable=1;
char acc_type[32]="None";
int acc_type_no=ACCELEROMETER_NONE;
int last_accel_time=0;
int accel_period=100;
int acc_rate=10;

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
		acc_type_no=ACCELEROMETER_MPU6050;
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
	
		acc_type_no=ACCELEROMETER_LSM303DLHC;
		acc_enable=1;
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
		if(millis()>(last_accel_time+accel_period))
		{
			sensors_event_t a;
			sensors_event_t g;
			sensors_event_t temp;
			
			switch(acc_type_no)
			{
				case ACCELEROMETER_NONE:		Serial.print("Accelerometer misconfigured, disabling\r\n");
												acc_enable=0;
												break;
				
				case ACCELEROMETER_MPU6050:		mpu.getEvent(&a,&g,&temp);
												
												Serial.print("Acceleration X: ");	Serial.print(a.acceleration.x);	Serial.print(", Y: ");	Serial.print(a.acceleration.y);	Serial.print(", Z: ");	Serial.print(a.acceleration.z);	Serial.println(" m/s^2\t");
	#if 0
												Serial.print("Rotation X: ");		Serial.print(g.gyro.x);			Serial.print(", Y: ");	Serial.print(g.gyro.y);			Serial.print(", Z: ");	Serial.print(g.gyro.z);			Serial.println(" rad/s\t");
												Serial.print("Temperature: ");		Serial.print(temp.temperature);	Serial.println(" degC\t");
	#endif
											
												break;

				case ACCELEROMETER_LSM303DLHC:	accel.getEvent(&a);
												
												Serial.print("Acceleration X: ");	Serial.print(a.acceleration.x);	Serial.print(", Y: ");	Serial.print(a.acceleration.y);	Serial.print(", Z: ");	Serial.print(a.acceleration.z);	Serial.println(" m/s^2\t");
												
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
			
			last_accel_time=millis();
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
	sensors_event_t a,g,temp;
	
	switch(cmd[1]|0x20)
	{
		case 'r':	Serial.print("Read accelerometer:\t");
					

					switch(acc_type_no)
					{
						case ACCELEROMETER_MPU6050:		mpu.getEvent(&a,&g,&temp);
														Serial.print("Acceleration X: ");	Serial.print(a.acceleration.x);	Serial.print(", Y: ");	Serial.print(a.acceleration.y);	Serial.print(", Z: ");	Serial.print(a.acceleration.z);	Serial.println(" m/s^2\t");
														break;

						case ACCELEROMETER_LSM303DLHC:	accel.getEvent(&a);
														Serial.print("Acceleration X: ");	Serial.print(a.acceleration.x);	Serial.print(", Y: ");	Serial.print(a.acceleration.y);	Serial.print(", Z: ");	Serial.print(a.acceleration.z);	Serial.println(" m/s^2\t");
														break;
			
						case ACCELEROMETER_MPU9250:		Serial.print("Accelerometer type not supported yet, disabling\r\n");
														acc_enable=0;
														break;
			
						case ACCELEROMETER_ADXL345:		Serial.print("Accelerometer type not supported yet, disabling\r\n");
														acc_enable=0;
														break;

						default:						Serial.print("Accelerometer disabled\r\n");
														break;
					}
					
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

