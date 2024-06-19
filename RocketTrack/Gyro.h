
#pragma once

extern bool gyro_enable;
extern bool gyro_trigger;

extern char gyro_type[];
extern int gyro_rate;

int SetupGyro(void);
void PollGyro(void);

void ReadGyro(float *gyro_x,float *gyro_y,float *gyro_z);

int GyroCommandHandler(uint8_t *cmd,uint16_t cmdptr);

enum
{
	GYRO_NONE=0,
	GYRO_MPU6050,
	GYRO_MPU9250,
	GYRO_L3GD20
};

