
#pragma once

typedef struct
{
	float gps_latitude;
	float gps_longitude;
	float gps_altitude;
	float gps_hdop;
	int gps_fix;
	int gps_numsats;
	float gps_max_altitude;
	float gps_launch_latitude;
	float gps_launch_longitude;
	float gps_apogee_latitude;
	float gps_apogee_longitude;
	float gps_landing_latitude;
	float gps_landing_longitude;
		
	float baro_altitude;
	float baro_pressure;
	float baro_temperature;
	float baro_humidity;
	float baro_max_altitude;
	float baro_launch_altitude;
	
	float accel_x;
	float accel_y;
	float accel_z;
	
	float gyro_x;
	float gyro_y;
	float gyro_z;
	
	float mag_x;
	float mag_y;
	float mag_z;

	float battery_voltage;
	float battery_current;

	float received_altitude;
	int packet_count;
	
} SensorState;

extern SensorState ss;

