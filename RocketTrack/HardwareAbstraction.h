
#pragma once

#ifdef ARDUINO_TBEAM_USE_RADIO_SX1276

	#pragma message "Building for TTGO T-Beam"

	#define UART_TXD			12
	#define UART_RXD			34

	#define SCK					5		// GPIO5  -- SX1278's SCK
	#define MISO				19		// GPIO19 -- SX1278's MISO
	#define MOSI				27		// GPIO27 -- SX1278's MOSI

	#define LORA_NSS			18
	#define LORA_RESET			14
	#define LORA_DIO0			26
	
	#define SDCARD_NSS			23

	#define SDA					21
	#define SCL					22

	#define USER_BUTTON			38

	#define GPS_BAUD_RATE		9600
	#define GPS_1PPS			37

	#define PMIC_IRQ			35

	#define USE_OLED_DISPLAY	1
	
#elif BOARD_FEATHER

	#define LED_PIN				13
	#define LED_ON				HIGH
	#define LED_OFF				!LED_ON

	#define SCK					11
	#define MISO				13
	#define MOSI				12
	
	#define SDA					17
	#define SCL					18
	
	#define USER_BUTTOIN		-1

	#define GPS_BAUD_RATE		9600
	#define GPS_1PPS			5		// GPIO5
	
	#define USE_OLED_DISPLAY	0

#elif BOARD_TTGO_OLED


#elif ARDUINO_XIAO_ESP32S3

	#warning "building for Xiao Esp32s3 board"

	#define UART_TXD			43
	#define UART_RXD			44

	#define SCK					7
	#define MISO				8
	#define MOSI				9

	#define LORA_NSS			41
	#define LORA_RESET			42
	#define LORA_BUSY			40
	#define LORA_DIO0			39
	
	#define SDCARD_NSS			-1

	#define SDA					5
	#define SCL					6

	#define USER_BUTTON			0

	#define LED_PIN				21
	#define LED_OFF				HIGH
	#define LED_ON				LOW

	#define GPS_BAUD_RATE		9600
	#define GPS_1PPS			-1

	#define PMIC_IRQ			-1
	
	#define USE_OLED_DISPLAY	0

#elif ARDUINO_HELTEC_WIRELESS_TRACKER

	#warning "building for the Heltec Tracker"

	#define UART_TXD			34
	#define UART_RXD			33

//	#define SCK					9
//	#define MISO				11
//	#define MOSI				10

	#define LORA_NSS			SS
	#define LORA_RESET			12
	#define LORA_BUSY			12
	#define LORA_DIO0			14
	
	#define SDCARD_NSS			-1

//	#define SDA					42
//	#define SCL					41

	#define USER_BUTTON			0

	#define LED_PIN				LED_BUILTIN
	#define LED_OFF				HIGH
	#define LED_ON				LOW

	#define GPS_BAUD_RATE		115200
	#define GPS_1PPS			36
	#define GNSS_RST			35

	#define PMIC_IRQ			-1
	
	#define USE_OLED_DISPLAY	1

#else
	#error "Unsupported board selected!"
#endif
