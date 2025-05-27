
#pragma once

#include "Global.h"

int SetupLoRa(void);

void PollLoRa(void);

int LORACommandHandler(uint8_t *cmd,uint16_t cmdptr);

#define LORA_LONG_RANGE_MODE	0
#define LORA_HIGH_RATE_MODE		1

#if LORA_BAND==434
	#warning "Using 434 MHz frequencies"

	#define LORA_CH1			434.050
	#define LORA_CH2			434.150
	#define LORA_CH3			434.250
	#define LORA_CH4			434.350
	#define LORA_CH5			434.450
	#define LORA_CH6			434.550
#else
	#warning "Using 868 MHz frequencies"

	#define LORA_CH1			868.050
	#define LORA_CH2			868.150
	#define LORA_CH3			868.250
	#define LORA_CH4			868.350
	#define LORA_CH5			868.450
	#define LORA_CH6			868.550
#endif

#define LORA_HIGH_RATE_BW	125000
#define LORA_HIGH_RATE_SF	7
#define LORA_HIGH_RATE_CR	8

#define LORA_LONG_RANGE_BW	31250
#define LORA_LONG_RANGE_SF	12
#define LORA_LONG_RANGE_CR	8

extern double lora_freq;

extern int lora_id;

extern char lora_mode[];
extern int lora_crc;

extern int hr_bw;
extern int hr_sf;
extern int hr_cr;
extern int hr_period;

extern int lr_bw;
extern int lr_sf;
extern int lr_cr;
extern int lr_period;

extern bool lora_constant_transmit;

extern int tx_active;

