
#pragma once

//#include <xyzFloat.h>


#ifndef ARDUINO_XIAO_ESP32S3
    #include "MPU6050_WE.h"
    #include "MPU6500_WE.h"
    #include "MPU9250_WE.h"
#else
	#include "ICM20948_WE.h"
#endif

int SetupIMU(void);
void PollIMU(void);

int IMUCommandHandler(uint8_t *cmd,uint16_t cmdptr);

extern bool imu_enable;

typedef enum
{
    NO_SENSOR=0,
    QMC5883L,
    HMC5883L,
    MPU_6500,
    MPU_6050,
    MPU_9250,
    ICM_20948,
    BMP_180,
    BME_180,
    BME_280,
    BME_680
} sensors;

extern float roll;
extern float pitch;
extern float yaw;
extern float heading;

extern float Mag_A11,Mag_A12,Mag_A13;
extern float Mag_A21,Mag_A22,Mag_A23;
extern float Mag_A31,Mag_A32,Mag_A33;
extern float Mag_B1,Mag_B2,Mag_B3;

extern xyzFloat AccelOffset;
extern xyzFloat AccelScale;
extern xyzFloat GyroOffset;