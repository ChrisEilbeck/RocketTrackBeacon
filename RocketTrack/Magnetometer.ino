








#include <Adafruit_LSM303DLH_Mag.h>
#include <Adafruit_Sensor.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303DLH_Mag_Unified mag = Adafruit_LSM303DLH_Mag_Unified(12345);

int mag_enable=1;

char mag_type[32]="Generic";

int mag_rate=200;

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
			// There was a problem detecting the LSM303 ... check your connections
			Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
			while (1)
			{
				;
			}
		}
		
		// Display some basic information on this sensor
		displaySensorDetails();
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

}


void displaySensorDetails(void)
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
	
	delay(500);
}