
#include "Packetisation.h"

uint32_t next_transmit=0;

int SetupScheduler(void)
{
	pinMode(USER_BUTTON,INPUT);
	if(digitalRead(USER_BUTTON)==0)
		lora_constant_transmit=true;
	
	return(0);
}

bool last_user_button=true;
int button_timer=0;

void PollScheduler(void)
{
#if (USER_BUTTON>=0)
	bool user_button=digitalRead(USER_BUTTON);
	
	if(!user_button&&last_user_button)
	{
		Serial.println("Button pressed");
		button_timer=millis();
	}
	
	if(user_button&&!last_user_button)
	{
		Serial.println("Button released");
		
		if(millis()>(button_timer+5000))
		{
			if(strcmp(lora_mode,"High Rate")==0)	strcpy(lora_mode,"Long Range");
			else									strcpy(lora_mode,"High Rate");
			
			SetLoRaMode(lora_mode);
			Serial.print("Toggling lora_mode to ");
			Serial.println(lora_mode);
		}
		else if(millis()>(button_timer+1000))
		{
			lora_constant_transmit=!lora_constant_transmit;
			Serial.print("Setting constant transmit to ");
			Serial.println(lora_constant_transmit);
		}
	}
	
	last_user_button=user_button;
#endif
	
	if(short_button_press)
	{
		lora_constant_transmit=!lora_constant_transmit;
		Serial.print("Setting constant transmit to ");
		Serial.println(lora_constant_transmit);
		
		if(lora_constant_transmit)
		{
			// set the time for the next burst so it's in the correct
			// timeslot
			
			next_transmit=1000*(1+millis_1pps()/1000);
			next_transmit+=100*(lora_id%10);
			
			Serial.print("Constant transmit on in timeslot ");
			Serial.println(lora_id%10);
		}
		
		short_button_press=false;
	}
	
	if(long_button_press)
	{
		if(strcmp(lora_mode,"High Rate")==0)	strcpy(lora_mode,"Long Range");
		else									strcpy(lora_mode,"High Rate");
		
		SetLoRaMode(lora_mode);
		Serial.print("Toggling lora_mode to ");
		Serial.println(lora_mode);
		
		ShowModeChange();
		
		long_button_press=false;
	}
	
	if(millis_1pps()>=next_transmit)
	{
		if(		(LoRaTransmitSemaphore==0)
			&&	(lora_constant_transmit)	)
		{
			next_transmit=1000*(1+millis_1pps()/1000);
			next_transmit+=100*(lora_id%10);
		
#if 0
			if(strcmp(lora_mode,"Long Range")==0)
			{
				next_transmit=millis_1pps()+lr_period;
				led_control(0xf0f0f0f0,1);
			}
			else
			{
				next_transmit=millis_1pps()+hr_period;
				led_control(0xaaaaaaaa,1);
			}
#endif			
#if 1
			Serial.printf("millis_1pps() = %d\r\n",millis_1pps());
#endif
#if 1
			lastfix.gpsfix=gps.fixquality_3d;
			lastfix.numsats=gps.satellites;
			lastfix.longitude=gps.longitudeDegrees;
			lastfix.latitude=gps.latitudeDegrees;
			lastfix.height=gps.altitude;
			lastfix.accuracy=gps.HDOP;
			lastfix.voltage=4200.0;		

			PackPacket(TxPacket,&TxPacketLength);
			EncryptPacket(TxPacket);
#endif

			LoRaTransmitSemaphore=1;			
		}
	}
}

