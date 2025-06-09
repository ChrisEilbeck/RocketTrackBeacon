
#pragma once

extern int ticksemaphore;
extern unsigned long int ticktime_micros;
extern unsigned long int ticktime_millis;

void SetupOnePPS(void);
void OnePPS_adjust(void);

unsigned long int millis_1pps(void);
unsigned long int micros_1pps(void);

