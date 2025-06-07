
//
// this shoud automatically detect between a combination of an
// MPU6500/QMC5883/BME180, or a ICM20948 with a BME180, or a MPU9250 with a
// BME180
//

#include "IMU.h"
#include "NvMemory.h"
#include "Webserver.h"

// for sensor fusion
#include <MadgwickAHRS.h>

#ifdef ARDUINO_XIAO_ESP32S3
	#include "ICM20948_WE.h"
	
	#define ICM20948_ADDR	0x68
	ICM20948_WE imu=ICM20948_WE(ICM20948_ADDR);	
#else
	// also supports MPU6500 on GY-91 boards these days
	#include "MPU6050_WE.h"
	#include "MPU6500_WE.h"
	#include "MPU9250_WE.h"

	// for the HMC magnetometer
	#include <Adafruit_Sensor.h>
	#include <Adafruit_HMC5883_U.h>

	// IMU devices
	MPU6500_WE mpu6500;
	MPU6050_WE mpu6050;
	MPU9250_WE mpu9250;

	Adafruit_HMC5883_Unified hmc5883l=Adafruit_HMC5883_Unified(5883);
#endif

bool imu_enable=true;

// typical declination value for the south of the UK
float Declination=-1-1/60;

// for calibration of the sensors
xyzFloat accelmin;
xyzFloat accelmax;

xyzFloat AccelOffset;
xyzFloat AccelScale;
xyzFloat GyroOffset;

// soft iron correction matrix for magnetometer
float Mag_A11,Mag_A12,Mag_A13;
float Mag_A21,Mag_A22,Mag_A23;
float Mag_A31,Mag_A32,Mag_A33;

// hard iron correction vector for magnetometer
float Mag_B1,Mag_B2,Mag_B3;

Madgwick filter;
float imu_rate=10.0;	// Hz

bool use_compass=true;
bool compass_live_mode=false;
bool mag_cal_mode=false;

int accel_setup=NO_SENSOR;
int gyro_setup=NO_SENSOR;
int magnetometer_setup=NO_SENSOR;

// Magdwick filter outputs for global use
float roll=0.0;
float pitch=0.0;
float yaw=0.0;
float heading=0.0;

void ResetCalibration(void);

int SetupIMU(void)
{
	Serial.print("SetupIMU()\r\n");
	
	bool fail=true;
	int cnt=0;

	fail=DetectSeparateBoards();
	
	if(fail)
	{
		Serial.println("MPU connection failed");
		use_compass=false;
		return(1);
	}
	
	filter.begin(imu_rate);
	
#if 0
	// identity matrix i.e. no correction
	Mag_A11=1.0;			Mag_A12=0.0;			Mag_A13=0.0;
	Mag_A21=0.0;			Mag_A22=1.0;			Mag_A23=0.0;
	Mag_A31=0.0;			Mag_A32=0.0;			Mag_A33=1.0;

	mag_B1=0.0;				Mag_B2=0.0;				Mag_B3=0.0;
#endif
#if 0
	Serial.println("\tApply magnetometer calibration for 4 element Yagi");

	// soft iron calibration matrix for 4 element Yagi antenna
	Mag_A11=9.7436e-01;		Mag_A12=-2.2516e-03;	Mag_A13=-7.3596e-02;
	Mag_A21=-2.2516e-03;	Mag_A22=1.1269e+00;		Mag_A23-3.0113e-03;
	Mag_A31=-7.3596e-02;	Mag_A32=-3.0113e-03;	Mag_A33=9.9677e-01;
	
	// hard iron calibration vector for 4 element Yagi antenna
	Mag_B1=-10.6583;		Mag_B2=8.0037;			Mag_B3=34.7096;
#endif
#if 1
	PrintMagCalibration();
#endif

	Serial.print("SetupIMU() complete ...\r\n");
	
	return(0);
}

void PrintMagCalibration(void)
{
	Serial.println("\n\tSoft iron calibration matrix");
	Serial.printf("\t\tA11=%.6f,\tA12=%.6f,\tA13=%.6f\r\n",Mag_A11,Mag_A12,Mag_A13);
	Serial.printf("\t\tA21=%.6f,\tA22=%.6f,\tA23=%.6f\r\n",Mag_A21,Mag_A22,Mag_A23);
	Serial.printf("\t\tA31=%.6f,\tA32=%.6f,\tA33=%.6f\r\n\n",Mag_A31,Mag_A32,Mag_A33);
	Serial.println("\tHard iron calibration vector");
	Serial.printf("\t\tB1=%.6f,\tB2=%.6f,\tB3=%.6f\r\n\n",Mag_B1,Mag_B2,Mag_B3);
}

bool DetectSeparateBoards(void)
{
	bool fail=false;
	
	Serial.println("\tSetting up individual motion sensor boards ...");
	
#ifndef ARDUINO_XIAO_ESP32S3
	if(accel_setup==NO_SENSOR)
	{
		if(mpu6050.init())
		{
			Serial.println("\t\tMPU6050 gyro/accelerometer initialised ...");
			accel_setup=MPU_6050;
			gyro_setup=MPU_6050;
		}
	}

	if(accel_setup==NO_SENSOR)
	{
		if(mpu6500.init())
		{
			Serial.println("\t\tMPU6500 gyro/accelerometer initialised ...");
			accel_setup=MPU_6500;
			gyro_setup=MPU_6500;
		}
	}

	if(accel_setup==NO_SENSOR)
	{
		if(mpu9250.init())
		{
			Serial.println("\t\tMPU9250 gyro/accelerometer initialised ...");
			accel_setup=MPU_9250;
			gyro_setup=MPU_9250;
		}
	}
	
	if(accel_setup==NO_SENSOR)
	{
		Serial.println("No accelerometer/gyro detected ...");
	}
	
	if(!hmc5883l.begin())
	{
		Serial.println("\t\tHMC5883 setup failed");
	} 
	else
	{
		Serial.println("\t\tHMC5883L Magnetometer initialised ...");
		magnetometer_setup=HMC5883L;
	}
#endif
		
	if((accel_setup!=NO_SENSOR)&&(gyro_setup!=NO_SENSOR)&&(magnetometer_setup!=NO_SENSOR))
		return(false);
	else
		return(true);
}

bool DetectCombinedBoard(void)
{
	bool fail=false;
	
	Serial.println("\tSetting up combined motion sensor board ...");


#ifdef ARDUINO_XIAO_ESP32S3
	if(!imu.init())
	{
    	Serial.println("ICM20948 does not respond");
    	fail=true;
	}
	else
	{
		Serial.println("ICM20948 is connected");
	}
#endif
	
	return(fail);
}

void ReadMagnetometer(float *x,float *y,float *z)
{
#if DEBUG>2
	Serial.println("magnetometer ...");
#endif

	switch(magnetometer_setup)
	{
#ifndef ARDUINO_XIAO_ESP32S3
		case HMC5883L:	{
							sensors_event_t event; 
							hmc5883l.getEvent(&event);
							
							*x=event.magnetic.x;
							*y=event.magnetic.y;
							*z=event.magnetic.z;
						}		
		
						break;
#endif
		
		default:		Serial.println("Magnetometer not configured!");
	}
}

void ReadAccelerometerGyro(float *ax,float *ay,float *az,float *gx,float *gy,float *gz)
{
#if DEBUG>2
	Serial.println("accelerometer/gyro ...");
#endif

	xyzFloat accel;
	xyzFloat gyro;

#ifndef ARDUINO_XIAO_ESP32S3
	switch(accel_setup)
	{
		case MPU_6050:	accel=mpu6050.getGValues();
						break;
		
		case MPU_6500:	accel=mpu6500.getGValues();
						break;
		
		case MPU_9250:	accel=mpu9250.getGValues();
						break;
		
		default:		Serial.println("Accelerometer not configured!");
	}

	switch(gyro_setup)
	{
		case MPU_6050:	gyro=mpu6050.getGyrValues();
						break;
		
		case MPU_6500:	gyro=mpu6500.getGyrValues();
						break;
		
		case MPU_9250:	gyro=mpu9250.getGyrValues();
						break;
		
		default:		Serial.println("Gyro not configured!");
	}
#else


#endif

	*ax=accel.x;
	*ay=accel.y;
	*az=accel.z;
	*gx=gyro.x;
	*gy=gyro.y;
	*gz=gyro.z;
}

void CorrectAccelerometer(float *x,float *y,float *z)
{
	
	
	
	
	
}

void CorrectGyro(float *x,float *y,float *z)
{
	*x+=GyroOffset.x;
	*y+=GyroOffset.y;
	*z+=GyroOffset.z;
}

void PollIMU(void)
{
	if(!imu_enable)
		return;

	if(!use_compass)
		return;

	if((accel_setup==NO_SENSOR)||(gyro_setup==NO_SENSOR)||(magnetometer_setup==NO_SENSOR))
		return;

	static int update_filter_at=0;
	
	xyzFloat accel;
	xyzFloat gyro;
	
	xyzFloat uncalibrated_mag;
	xyzFloat calibrated_mag;
	
	if(millis()>update_filter_at)
	{
		update_filter_at=millis()+(int)(1000/imu_rate);
	
		ReadMagnetometer(
							&(uncalibrated_mag.x),
							&(uncalibrated_mag.y),
							&(uncalibrated_mag.z)
						);
		
		// see https://teslabs.com/articles/magnetometer-calibration/ and 
		// https://sailboatinstruments.blogspot.com/2011/08/improved-magnetometer-calibration.html
		// for a better explanation than I have of all of this 
		
		xyzFloat mag_temp;
		
		// apply the mag offsets i.e. remove hard iron distortion
		mag_temp.x=uncalibrated_mag.x-Mag_B1;
		mag_temp.y=uncalibrated_mag.y-Mag_B2;
		mag_temp.z=uncalibrated_mag.z-Mag_B3;
		
		// apply the soft iron distortion matrix
		calibrated_mag.x=mag_temp.x*Mag_A11+mag_temp.y*Mag_A12+mag_temp.z*Mag_A13;
		calibrated_mag.y=mag_temp.x*Mag_A21+mag_temp.y*Mag_A22+mag_temp.z*Mag_A23;
		calibrated_mag.z=mag_temp.x*Mag_A31+mag_temp.y*Mag_A32+mag_temp.z*Mag_A33;
		
		// remap the axes to be the same as the gyro and accelerometer
		mag_temp.x=calibrated_mag.x;	mag_temp.y=calibrated_mag.y;	mag_temp.z=calibrated_mag.z;		
		calibrated_mag.x=mag_temp.y;	calibrated_mag.y=-mag_temp.x;	calibrated_mag.z=mag_temp.z;
		
		// these are values scaled to proper units
		ReadAccelerometerGyro(
								&(accel.x),&(accel.y),&(accel.z),
								&(gyro.x),&(gyro.y),&(gyro.z)
							);
		
		// correct using the calibration values
		CorrectAccelerometer(&(accel.x),&(accel.y),&(accel.z));
		CorrectGyro(&(gyro.x),&(gyro.y),&(gyro.z));
		
		filter.update(
						gyro.x,gyro.y,gyro.z,
						accel.x,accel.y,accel.z,
						calibrated_mag.x,calibrated_mag.y,calibrated_mag.z
					);
		
		heading=180.0-filter.getYaw();
		if(heading<0.0)		heading+=360.0;
		if(heading>360.0)	heading-=360.0;
		
		if(mag_cal_mode)
		{
			static int update_ui_at=0;
			
			if(millis()>update_ui_at)
			{
				update_ui_at=millis()+100;
				
				if(	(uncalibrated_mag.x!=0.0)&&(uncalibrated_mag.y!=0.0)&&(uncalibrated_mag.z!=0.0)	)
					Serial.printf("MagCal:%.3f,%.2f,%.2f,%.2f\r\n",(float)millis()/1000.0,uncalibrated_mag.x,uncalibrated_mag.y,uncalibrated_mag.z);
			}
		}
		
		if(compass_live_mode)
		{
			static int update_ui_at=0;
			
			if(millis()>update_ui_at)
			{
				update_ui_at=millis()+100;
		
#if 0
				Serial.printf("AccX: % .2f, AccY: % .2f, AccZ: % .2f\t",accel.x,accel.y,accel.z);
				Serial.printf("GyroX: % .2f, GyroY: % .2f, GyroZ: % .2f\t",gyro.x,gyro.y,gyro.z);
				Serial.printf("MagX: % .2f, MagY: % .2f, MagZ: % .2f, Heading: %.2f\r\n",uncalibrated_mag.x,uncalibrated_mag.y,uncalibrated_mag.z,heading);
#endif
#if 1
				Serial.printf("MagX: % .2f, MagY: % .2f, MagZ: % .2f, AccX: %.2f, AccY: %.2f, AccZ: %.2f, GyroX: %.2f, GyroY: %.2f, GyroZ: %.2f\r\n",
									uncalibrated_mag.x,uncalibrated_mag.y,uncalibrated_mag.z,
									accel.x,accel.y,accel.z,
									gyro.x,gyro.y,gyro.z
							);
#endif
#if 0
				Serial.printf("MagX: % .2f, MagY: % .2f, MagZ: % .2f, Cal_MagX: % .2f, Cal_MagY: % .2f, Cal_MagZ: % .2f\r\n",
									uncalibrated_mag.x,uncalibrated_mag.y,uncalibrated_mag.z,
									calibrated_mag.x,calibrated_mag.y,calibrated_mag.z
							);
#endif
#if 0
				Serial.printf("Cal_MagX: % .2f, Cal_MagY: % .2f, Cal_MagZ: % .2f, ",calibrated_mag.x,calibrated_mag.y,calibrated_mag.z);
				Serial.printf("Roll: %.1f, Pitch: %.1f, Yaw: %.1f, Heading: %.1f\r\n",filter.getRoll(),filter.getPitch(),filter.getYaw(),heading);
#endif
			}
		}
	}
}

int IMUCommandHandler(uint8_t *cmd,uint16_t cmdptr)
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
		case 'h':	// compass heading
					Serial.printf("Heading: %.1f\r\n",heading);
					break;
		
		case 'p':	// pitch
					Serial.printf("Pitch: %.1f\r\n",pitch);
					break;
		
		case 'r':	// roll
					Serial.printf("Roll: %.1f\r\n",roll);
					break;
		
		case 'y':	// yaw
					Serial.printf("Yaw: %.1f\r\n",yaw);
					break;
		
		case 'l':	// live mode toggle
					compass_live_mode=!compass_live_mode;
					
					if(compass_live_mode)	Serial.print("Enabling live compass data mode\r\n");
					else					Serial.print("Disabling live compass data mode\r\n");
					
					break;
		
		case 'c':	// calibration mode toggle
					mag_cal_mode=!mag_cal_mode;
					
					if(mag_cal_mode)		Serial.print("Enabling magnetometer calibration mode\r\n");
					else					Serial.print("Disabling magnetometer calibration mode\r\n");
					
					break;
		
		case '?':	Serial.print("IMU Test Harness\r\n================\r\n\n");
					Serial.print("c\t-\tEnable/disable magnetometer calibration mode\r\n");
					Serial.print("h\t-\tDisplay the compass heading\r\n");
					Serial.print("l\t-\tEnable/disable live compass data display\r\n");
					Serial.print("p\t-\tRead current pitch value\r\n");
					Serial.print("r\t-\tRead current roll value\r\n");
					Serial.print("y\t-\tRead current yaw value\r\n");
					Serial.print("?\t-\tShow this menu\r\n");
					break;
		
		default:	retval=0;
					break;
	}
	
	return(retval);
}

void CalibrateAccelerometer(char *direction,float *output,char axis)
{
	xyzFloat accel;
	xyzFloat gyro;
	
	Serial.print("\tPlace with ");
	Serial.println(direction);
	
	Serial.print("3 ...");				delay(1000);
	Serial.print("2 ...");				delay(1000);
	Serial.print("1 ...\r\n");			delay(1000);
	Serial.println("\tMeasuring ...");

	ReadAccelerometerGyro(
							&(accel.x),&(accel.y),&(accel.z),
							&(gyro.x),&(gyro.y),&(gyro.z)
						);
	
	switch(axis)
	{
		case 'x':	*output=accel.x;
					break;
		
		case 'y':	*output=accel.y;
					break;
		
		case 'z':	*output=accel.z;
					break;
	
		default:	Serial.printf("Invalid axis selected in CalibrateAccelerometer() - \'%c\'\r\n",axis);
	}
	
	ComputeAccelOffsetAndScale();
}

void ComputeAccelOffsetAndScale(void)
{
	Serial.print("X:\tmax = ");	Serial.print(accelmax.x);	Serial.print("\tmin = ");	Serial.println(accelmin.x);
	Serial.print("Y:\tmax = ");	Serial.print(accelmax.y);	Serial.print("\min = t");	Serial.println(accelmin.y);
	Serial.print("Z:\tmax = ");	Serial.print(accelmax.z);	Serial.print("\tmin = ");	Serial.println(accelmin.z);

	AccelOffset.x=accelmax.x+accelmin.x;
	AccelOffset.y=accelmax.y+accelmin.y;
	AccelOffset.z=accelmax.z+accelmin.z;

	AccelScale.x=2.0/(accelmax.x-accelmin.x);
	AccelScale.y=2.0/(accelmax.y-accelmin.y);
	AccelScale.z=2.0/(accelmax.z-accelmin.z);
	
	Serial.print("\tXoff = ");	Serial.print(AccelOffset.x);	Serial.print("\tXscale = ");	Serial.println(AccelScale.x);
	Serial.print("\tYoff = ");	Serial.print(AccelOffset.y);	Serial.print("\tYscale = ");	Serial.println(AccelScale.y);
	Serial.print("\tZoff = ");	Serial.print(AccelOffset.z);	Serial.print("\tZscale = ");	Serial.println(AccelScale.z);
}

int SensorCalibrationCommandHandler(uint8_t *cmd,uint16_t cmdptr)
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
		case 'c':	// clear all calibration
					ResetCalibration();
					break;

		case 'g':	// calibrate the gyro
					CalibrateGyro();
					
					break;

		case 'p':	// print valibration values
					PrintCalibrationValues();
					break;

		case 's':	// store the calibration values
					StoreCalibrationData();
					break;
		
		case 't':	// retrieve the calibration values
					RetrieveCalibrationData();
					break;

		case 'x':	if(cmd[2]=='-')	CalibrateAccelerometer("Nose down",&(accelmin.x),'x');
					if(cmd[2]=='+')	CalibrateAccelerometer("Nose up",&(accelmax.x),'x');
							
					break;
		
		case 'y':	if(cmd[2]=='-')	CalibrateAccelerometer("Left down",&(accelmin.y),'y');
					if(cmd[2]=='+')	CalibrateAccelerometer("Right down",&(accelmax.y),'y');
					
					break;
		
		case 'z':	if(cmd[2]=='+')	CalibrateAccelerometer("Flat and level",&(accelmax.z),'z');
					if(cmd[2]=='-')	CalibrateAccelerometer("Upside-down",&(accelmin.z),'z');
					
					break;
		
		case '?':	Serial.print("IMU Test Harness\r\n================\r\n\n");
					Serial.print("g\t-\tCalibrate the Gyro\r\n");
					Serial.print("p\t-\tPrint calibration values\r\n");
					Serial.print("s\t-\tStore the calibration values\r\n");
					Serial.print("t\t-\tRetrieve the calibration values\r\n");
					Serial.print("x-\t-\tCalibrate X- accelerometer reading\r\n");
					Serial.print("x+\t-\tCalibrate X+ accelerometer reading\r\n");
					Serial.print("y-\t-\tCalibrate Y- accelerometer reading\r\n");
					Serial.print("y+\t-\tCalibrate Y+ accelerometer reading\r\n");
					Serial.print("z-\t-\tCalibrate Z- accelerometer reading\r\n");
					Serial.print("z+\t-\tCalibrate Z+ accelerometer reading\r\n");
					Serial.print("?\t-\tShow this menu\r\n");
					break;
		
		default:	retval=0;
					break;
	}
	
	return(retval);
}

void ResetCompassCalibration(void)
{
}

void CalibrateGyro(void)
{
	Serial.println("CalibrateGyro() entry");
	Serial.println("\tPlace the unit on a flat surface and do not move it for 3 seconds ...");
	
	delay(1000);
	
	Serial.println("\tMeasuring ...");
	
	int now=millis();
	xyzFloat sum;
	int cnt=0;
	
	while((millis()-now)<3000)
	{
		xyzFloat accel;
		xyzFloat gyro;
		
		ReadAccelerometerGyro(
								&(accel.x),&(accel.y),&(accel.z),
								&(gyro.x),&(gyro.y),&(gyro.z)
							);
	
		sum+=gyro;
		cnt++;
		delay(10);
	}
	
	sum/=(float)cnt;
	
	Serial.printf("\tMeasured X: %.1f, Y: %.1f, Z: %.1f\r\n",sum.x,sum.y,sum.z);	
	
	GyroOffset.x=-sum.x;	GyroOffset.y=-sum.y;	GyroOffset.z=-sum.z;
		
	Serial.println("CalibrateGyro() exit");
}

void PrintCalibrationValues(void)
{
	Serial.print("Accelerometer:\r\n");
	Serial.printf("\tX: %.1f\tto\t%.1f\r\n",accelmin.x,accelmax.x);
	Serial.printf("\tY: %.1f\tto\t %.1f\r\n",accelmin.y,accelmax.y);
	Serial.printf("\tZ: %.1f\tto\t%.1f\r\n\n",accelmin.z,accelmax.z);
	
	Serial.print("Gyroscope:\r\n");
	Serial.printf("\tX: %.1f\r\n",GyroOffset.x);
	Serial.printf("\tY: %.1f\r\n",GyroOffset.y);
	Serial.printf("\tZ: %.1f\r\n\n",GyroOffset.z);

}

void ResetCalibration(void)
{
	accelmin.x=-16384.0;	accelmax.x=16384.0;
	accelmin.y=-16384.0;	accelmax.y=16384.0;
	accelmin.z=-16384.0;	accelmax.z=16384.0;
	
	GyroOffset.x=0.0;	GyroOffset.y=0.0;	GyroOffset.z=0.0;					
}

