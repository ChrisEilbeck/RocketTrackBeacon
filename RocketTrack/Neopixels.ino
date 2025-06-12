
#if 0
bool neopixels_enable=true;
uint32_t NextNeopixels=0;

uint32_t NeopixelPattern[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint16_t NeopixelRepeatCount=0;
uint8_t NeopixelBitCount=0;

uint32_t NeopixelTestPattern[32]={0x00000000,0x00200000,0x00400000,0x00600000,0x00800000,0x00a00000,0x00c00000,0x00f00000,0x00c02000,0x00a04000,0x00806000,0x00608000,0x0040a000,0x0020c000,0x0000f000,0x0000c020,0x0000a040,0x00008060,0x00006080,0x000040a0,0x000020c0,0x000000f0,0x000000c0,0x000000a0,0x00000080,0x00000060,0x00000040,0x00000020,0x00000000,0x00000000,0x00000000,0x00000000};

#include <Adafruit_NeoPixel.h>

//define NeoPixel Pin and Number of LEDs
#define PIN 5
#define NUM_LEDS 3

Adafruit_NeoPixel strip=Adafruit_NeoPixel(NUM_LEDS,PIN,NEO_GRB+NEO_KHZ800);

int SetupNeopixels(void)
{
	if(neopixels_enable)
	{
		strip.begin();
		strip.show();
	}
	
	return(0);
}

void PollNeopixels(void)
{
	if(neopixels_enable)
	{
		if(millis()>=NextLEDs)
		{
			for(int cnt=0;cnt<NUM_LEDS;cnt++)
			{
				strip.setPixelColor(cnt,(NeopixelPattern[0]&0x00ff0000)>>16,(NeopixelPattern[0]&0x0000ff00)>>8,NeopixelPattern[0]&0x000000ff);
			}
			
			strip.show();
			
			uint32_t temp=NeopixelPattern[0];
			memcpy(NeopixelPattern,NeopixelPattern+4,31*4);

			if(NeopixelRepeatCount>0)		NeopixelPattern[31]=temp;
			else							NeopixelPattern[31]=0x00000000;
			
			NeopixelBitCount++;
			if(NeopixelBitCount>=32)
			{
				if(NeopixelRepeatCount>0)	NeopixelRepeatCount--;
				NeopixelBitCount=0;
			}
						
			// run the neopixels on a 5Hz cycle
			NextNeopixels=millis()+200L;
		}
	}
}

int NeopixelCommandHandler(uint8_t *cmd,uint16_t cmdptr)
{
	// ignore a single key stroke
	if(cmdptr<=2)	return(0);

#if (DEBUG>0)
	Serial.println((char *)cmd);
#endif
	
	int retval=1;
	
	switch(cmd[1]|0x20)
	{
		case 't':	Serial.print("Triggering Neopixel Test pattern\r\n");
					memcpy(NeopixelPattern,NeopixelTestPattern,sizeof(NeopixelTestPattern));
					NeopixelRepeatCount=3;
					NeopixelBitCount=0;
					retval=0;
					break;
		
		default:	// ignore
					break;
	}
	
	return(retval);
}

#endif
