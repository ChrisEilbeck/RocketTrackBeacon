
#include "HardwareAbstraction.h"

bool firsttime=true;

int ticksemaphore=0;
unsigned long int ticktime_micros=0;
unsigned long int ticktime_millis=0;

long int millis_offset=0;
long int micros_offset=0;

#define NUMPTS	1

void SetupOnePPS(void)
{
#if(GPS_1PPS>-1)
	pinMode(GPS_1PPS,INPUT_PULLDOWN);
	attachInterrupt(GPS_1PPS,OnePPSInterrupt,RISING);
#endif
}

void OnePPSInterrupt(void)
{
	ticktime_micros=micros();
	ticktime_millis=millis();
	ticksemaphore=1;
}

void PollOnePPS(void)
{
	if(ticksemaphore)
	{
#if 0
		Serial.print("tick\r\n");
#endif
				
		OnePPS_adjust();
		ticksemaphore=0;
	}
}

void OnePPS_adjust(void)
{	
	static unsigned long int last_millis;
	static unsigned long int last_micros;
		
	if(firsttime)
	{
		micros_offset=micros()%1000000;
		millis_offset=1000+millis()%1000;
		
		last_millis=ticktime_millis-1000;
		last_micros=ticktime_micros-1000000;
		
		firsttime=false;
		
		return;
	}

	static long int rect[NUMPTS]={0};
	static int cnt=0;
	
#if 0
	Serial.print("tick\t");
#endif

	// adjust the micros() timer
	
	int slippage=(long int)ticktime_micros-(long int)last_micros-1000000;
	
	// this is to cope with potentially missing a pulse or two if the GPS
	// has lost lock
	
	while(slippage<-10000)	slippage+=1000000;
	while(slippage>10000)	slippage-=1000000;
	
	rect[cnt]=slippage;

#if 0
	Serial.print(slippage);
	Serial.print("\t\t");
#endif
	
	long int mean=0;
	for(int i=0;i<NUMPTS;i++)
		mean+=rect[i];
	
	mean/=NUMPTS;
	micros_offset+=mean;

	cnt++;
	if(cnt>=NUMPTS)
		cnt=0;

	last_micros=ticktime_micros;
	
#if 0
	Serial.print("micros_offset = ");
	Serial.print(micros_offset);
	Serial.print("\t");
#endif
		
	// adjust the millis() timer
	
#if 0
	Serial.print(ticktime_millis-last_millis-1000);
	Serial.print("\t");
#endif
	
	millis_offset+=ticktime_millis-last_millis-1000;
	last_millis=ticktime_millis;

#if 0
	Serial.print("millis_offset = ");
	Serial.print(millis_offset);
	Serial.print("\t");
#endif
#if 0
	Serial.print(micros_1pps());
	Serial.print("\t");
#endif
#if 0
	Serial.print(millis_1pps());
#endif
#if 0
	Serial.println("");
#endif
}

unsigned long int millis_1pps(void)
{
	return(millis()-millis_offset);
}

unsigned long int micros_1pps(void)
{
	return(micros()-micros_offset);
}

