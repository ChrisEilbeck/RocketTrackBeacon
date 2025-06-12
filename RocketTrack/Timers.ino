
#if !defined(ARDUINO_RASPBERRY_PI_PICO)

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
//#include "ESP32TimerInterrupt.h"

#include "Timers.h"

int timer_enable=1;

#if !defined( ESP32 )
  #error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif


#if 0

ESP32Timer ITimer0(0);

int SetupTimers(void)
{
	Serial.print("Starting timer interrupts on ");	Serial.println(ARDUINO_BOARD);
	Serial.println(ESP32_TIMER_INTERRUPT_VERSION);
	Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));

	// Using ESP32  => 80 / 160 / 240MHz CPU clock ,
	// For 64-bit timer counter
	// For 16-bit timer prescaler up to 1024

	// Interval in microsecs
	if(ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0))
	{
		Serial.print(F("Starting  ITimer0 OK, millis() = ")); Serial.println(millis());
	}
	else
	{
		Serial.println(F("Can't set ITimer0. Select another Timer, freq. or timer"));
		return(1);
    }
    
    return(0);
}

#endif

#endif
