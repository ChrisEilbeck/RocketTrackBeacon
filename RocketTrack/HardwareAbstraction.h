
#pragma once

#if ARDUINO_TBEAM_USE_RADIO_SX1276

	#pragma message "Building for TTGO T-Beam"

	#define LED_PIN			-1
	#define LED_ON			HIGH
	#define LED_OFF			!LED_ON

	#define SCK				5		// GPIO5  -- SX1278's SCK
	#define MISO			19		// GPIO19 -- SX1278's MISO
	#define MOSI			27		// GPIO27 -- SX1278's MOSI

	#define LORA_NSS		18
	#define LORA_RESET		14
	#define LORA_DIO0		26
	
	#define SDCARD_NSS		23

	#define SDA				21
	#define SCL				22

	#define USER_BUTTON		38

	#define GPS_BAUD_RATE	9600
	#define GPS_1PPS		37

	#define PMIC_IRQ		35

#elif ARDUINO_TTGO_LoRa32_v21new

	#pragma message "Building for TTGO Paxcounter V1.6"

	#define LED_PIN			25
	#define LED_ON			HIGH
	#define LED_OFF			!LED_ON

	#define SCK				5		// GPIO5  -- SX1278's SCK
	#define MISO			19		// GPIO19 -- SX1278's MISO
	#define MOSI			27		// GPIO27 -- SX1278's MOSI

	#define LORA_NSS		18
	#define LORA_RESET		23
	#define LORA_DIO0		26
	
	#define SDCARD_NSS		13

	#define SDA				21
	#define SCL				22

	#define USER_BUTTON		-1

	#define GPS_BAUD_RATE	9600
	#define GPS_1PPS		36

	#define PMIC_IRQ		-1

#elif BOARD_FEATHER

	#define LED_PIN			13
	#define LED_ON			HIGH
	#define LED_OFF			!LED_ON

	#define SCK				11
	#define MISO			13
	#define MOSI			12
	
	#define SDA				17
	#define SCL				18
	
	#define USER_BUTTOIN	-1

	#define GPS_BAUD_RATE	9600
	#define GPS_1PPS		5		// GPIO5
	
#else

	#error "Unsupported board selected!"

#endif
