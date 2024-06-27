
#include "HardwareAbstraction.h"

uint32_t next_transmit=0;

int SetupScheduler(void)
{
	if(USER_BUTTON>=0)
	{
		pinMode(USER_BUTTON,INPUT);
		if(digitalRead(USER_BUTTON)==0)
			lora_constant_transmit=true;
	}
	
#if USE_FREERTOS
	Serial.println("Creating scheduler tasks");

	xTaskCreate(ButtonTask,"Button Task",2048,NULL,2,NULL);
	xTaskCreate(TxSchedulerTask,"TxScheduler Task",2048,NULL,2,NULL);
#endif
	
	return(0);
}

bool last_user_button=true;
int button_timer=0;

void PollScheduler(void)
{
	if(USER_BUTTON<0)
		return;

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
	
	if(short_button_press)
	{
		lora_constant_transmit=!lora_constant_transmit;
		Serial.print("Setting constant transmit to ");
		Serial.println(lora_constant_transmit);
		
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
	
	if(millis()>=next_transmit)
	{
		Serial.println(LoRaTransmitSemaphore);
		Serial.println(lora_constant_transmit);
		
		if(		(LoRaTransmitSemaphore==0)
			&&	(lora_constant_transmit)	)
		{
			PackPacket(TxPacket,&TxPacketLength);
			EncryptPacket(TxPacket);
			LoRaTransmitSemaphore=1;
			
			if(strcmp(lora_mode,"Long Range")==0)
			{
				next_transmit=millis()+10000;
				led_control(0xf0f0f0f0,1);
			}
			else
			{
				next_transmit=millis()+200;
				led_control(0xaaaaaaaa,1);
			}
			
#if 0
			Serial.printf("millis() = %d\r\n",millis());
#endif
		}
	}
}

void ButtonTask(void *pvParameters)
{
	while(1)
	{
		if(USER_BUTTON>=0)
		{
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
			
			if(short_button_press)
			{
				lora_constant_transmit=!lora_constant_transmit;
				Serial.print("Setting constant transmit to ");
				Serial.println(lora_constant_transmit);
				
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
		}
				
		delay(1);
	}
}

void TxSchedulerTask(void *pvParameters)
{
	while(1)
	{
		if(lora_constant_transmit)
		{
			PackPacket(TxPacket,&TxPacketLength);
			EncryptPacket(TxPacket);
			LoRaTransmitSemaphore=1;
			
			if(strcmp(lora_mode,"Long Range")==0)
			{
				led_control(0xf0f0f0f0,1);
				delay(10000);
			}
			else
			{
				led_control(0xaaaaaaaa,1);
				delay(200);
			}
		}
	
		delay(1);
	}
}


