
#include <Adafruit_LSM303DLH_Mag.h>
#include <Adafruit_Sensor.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303DLH_Mag_Unified mag=Adafruit_LSM303DLH_Mag_Unified(10003);

int mag_enable=1;

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

void PollMagnetometer(void)
{
	if(mag_enable)
	{
		if(millis()>(last_mag_time+mag_period))
		{
			SampleMagnetometer();
			last_mag_time=millis();
		}
	}
}

void SampleMagnetometer(void)
{
	sensors_event_t event;
	mag.getEvent(&event);

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

