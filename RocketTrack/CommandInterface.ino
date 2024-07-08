
#include "CommandInterface.h"

void SetupCommandHandler(void)
{
#if USE_FREERTOS
	xTaskCreate(CommandHandlerTask,"Command Handler Task",2048,NULL,2,NULL);
#endif
}

void PollSerial(void)
{
	static uint8_t cmd[128];
	static uint16_t cmdptr=0;
	char rxbyte;

	while(Serial.available())
	{ 
		rxbyte=Serial.read();
		
		cmd[cmdptr++]=rxbyte;
		
		if((rxbyte=='\r')||(rxbyte=='\n'))
		{
			ProcessCommand(cmd,cmdptr);
			memset(cmd,0,sizeof(cmd));
			cmdptr=0;
		}
		else if(cmdptr>=sizeof(cmd))
		{
			cmdptr--;
		}
	}
}

void CommandHandlerTask(void *pvParameters)
{
	static uint8_t cmd[128];
	static uint16_t cmdptr=0;
	char rxbyte;
	
	while(1)
	{
		while(Serial.available())
		{ 
			rxbyte=Serial.read();
			
			cmd[cmdptr++]=rxbyte;
			
			if((rxbyte=='\r')||(rxbyte=='\n'))
			{
				ProcessCommand(cmd,cmdptr);
				memset(cmd,0,sizeof(cmd));
				cmdptr=0;
			}
			else if(cmdptr>=sizeof(cmd))
			{
				cmdptr--;
			}
		}
		
		delay(1);
	}
}

void ProcessCommand(uint8_t *cmd,uint16_t cmdptr)
{
	int OK=0;
	
	switch(cmd[0]|0x20)
	{
		case 'a':	OK=AccelerometerCommandHandler(cmd,cmdptr);		break;
		case 'b':	OK=BarometerCommandHandler(cmd,cmdptr);			break;
		case 'e':	OK=LEDCommandHandler(cmd,cmdptr);				break;
		case 'g':	OK=GPSCommandHandler(cmd,cmdptr);				break;
		case 'h':	OK=HighRateCommandHandler(cmd,cmdptr);			break;
		case 'l':	OK=LORACommandHandler(cmd,cmdptr);				break;
		case 'm':	OK=MagnetometerCommandHandler(cmd,cmdptr);		break;
		case 'n':	OK=NeopixelCommandHandler(cmd,cmdptr);			break;
		case 'o':	OK=LongRangeCommandHandler(cmd,cmdptr);			break;
#ifdef ARDUINO_TBEAM_USE_RADIO_SX1262		
		case 'p':	OK=PMICCommandHandler(cmd,cmdptr);				break;
#endif
		case 'y':	OK=GyroCommandHandler(cmd,cmdptr);				break;
		case 'z':	OK=BeeperCommandHandler(cmd,cmdptr);			break;
		
		case 'x':	OK=1;
					i2c_bus_scanner();
					break;

#if 0		
		case 't':	{
						char taskbuffer[2048];
						vTaskList(taskbuffer);
						Serial.println(taskbuffer);
					}
					
					break;
#endif
				
		case '?':	Serial.print("RocketTrack Test Harness Menu\r\n=================\r\n\n");
					Serial.print("a\t-\tAccelerometer Commands\r\n");
					Serial.print("b\t-\tBarometer Commands\r\n");
					Serial.print("e\t-\tLed Commands\r\n");
					Serial.print("g\t-\tGPS Commands\r\n");
					Serial.print("h\t-\tHigh Rate Mode Commands\r\n");
					Serial.print("l\t-\tLoRa Commands\r\n");
					Serial.print("m\t-\tMagnetometer Commands\r\n");
#ifdef ARDUINO_TBEAM_USE_RADIO_SX1262
					Serial.print("p\t-\tPMIC Commands\r\n");
#endif
//					Serial.print("n\t-\tNeopixel Commands\r\n");
					Serial.print("o\t-\tLong Range Mode Commands\r\n");
//					Serial.print("t\t-\tTransmitter Mode\r\n");
//					Serial.print("r\t-\tReceiver Mode\r\n");
					Serial.print("y\t-\tGyro Commands\r\n");
					Serial.print("?\t-\tShow this menu\r\n");
					OK=1;
					break;
					
		default:	// do nothing
					break;
	}

	if(OK)	{	Serial.println("ok ...");	}
	else	{	Serial.println("?");	}
}

void i2c_bus_scanner(void)
{
	byte error, address;
	int nDevices;
	
	Serial.println("Scanning...");
	
	nDevices = 0;
	for(address = 1; address < 127; address++ ) 
	{
		// The i2c_scanner uses the return value of
		// the Write.endTransmisstion to see if
		// a device did acknowledge to the address.
		Wire.beginTransmission(address);
		error = Wire.endTransmission();
		
		if (error == 0)
		{
			Serial.print("I2C device found at address 0x");
			if (address<16) 
				Serial.print("0");
			Serial.print(address,HEX);
			Serial.println("  !");

			nDevices++;
		}
		else if (error==4) 
		{
			Serial.print("Unknown error at address 0x");
			if (address<16) 
				Serial.print("0");
			Serial.println(address,HEX);
		}    
	}
	
	if (nDevices == 0)
		Serial.println("No I2C devices found\n");
	else
		Serial.println("done\n");

	//	I2C device found at address 0x34  !		// AXP192 PMIC
	//	I2C device found at address 0x3C  !		// OLED Display
	//	I2C device found at address 0x68  !		// MPU6050 Accelerometer/Magnetometer/Gyro
	//	I2C device found at address 0x76  !		// BME280 pressure sensor  
}


