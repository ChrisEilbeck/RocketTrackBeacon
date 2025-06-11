
#include "Display.h"
#include "Packetisation.h"

#include <Wire.h>

#include <Adafruit_GFX.h>

#ifdef ARDUINO_HELTEC_WIRELESS_TRACKER
	#include "HT_st7735.h"
	
	#define SCREEN_WIDTH	160
	#define SCREEN_HEIGHT	80

	HT_st7735 st7735;

	const uint16_t BORDER_COLOR = ST7735_RED;
	const uint16_t FONT_7x10_COLOR = ST7735_RED;
	const uint16_t FONT_11x18_COLOR = ST7735_GREEN;
	const uint16_t FONT_16x26_COLOR = ST7735_BLUE;
	const uint16_t BLACK_COLOR = ST7735_BLACK;
	const uint16_t BLUE_COLOR = ST7735_BLUE;
	const uint16_t RED_COLOR = ST7735_RED;
	const uint16_t GREEN_COLOR = ST7735_GREEN;
	const uint16_t CYAN_COLOR = ST7735_CYAN;
	const uint16_t MAGENTA_COLOR = ST7735_MAGENTA;
	const uint16_t YELLOW_COLOR = ST7735_YELLOW;
	const uint16_t WHITE_COLOR = ST7735_WHITE;
#else
	#include <Adafruit_SSD1306.h>
	
	#define SCREEN_WIDTH	128
	#define SCREEN_HEIGHT	64

	// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
	// The pins for I2C are defined by the Wire-library. 
	// On an arduino UNO:       A4(SDA), A5(SCL)
	// On an arduino MEGA 2560: 20(SDA), 21(SCL)
	// On an arduino LEONARDO:   2(SDA),  3(SCL), ...

	#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
	#define SCREEN_ADDRESS 0x3c ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

	Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire,OLED_RESET);
#endif

#define DISPLAY_UPDATE_PERIOD	1000

int display_update_suspend=0;

bool display_enable=false;

int SetupDisplay(void)
{
#ifdef ARDUINO_HELTEC_WIRELESS_TRACKER
	pinMode(Vext, OUTPUT);
	digitalWrite(Vext, HIGH); //LCD needs power before init.

	st7735.st7735_init();
 
	Serial.printf("st7735 display ready!\r\n");

    st7735.st7735_fill_screen(BLACK_COLOR);  

    st7735.st7735_write_str(32,12,"Rocket",Font_16x26,WHITE_COLOR,BLACK_COLOR);  
    st7735.st7735_write_str(40,42,"Track",Font_16x26,WHITE_COLOR,BLACK_COLOR);   
#else
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if(!display.begin(SSD1306_SWITCHCAPVCC,SCREEN_ADDRESS))
	{
		Serial.println(F("SSD1306 allocation failed"));
		return(1);
	}
	
	display_enable=true;
	
	Serial.println(F("SSD1306 display configured ..."));
	
	display.setRotation(2);
	
	display.clearDisplay();
	
	display.setTextSize(2);					// Normal 1:1 pixel scale
	display.setTextColor(SSD1306_WHITE);	// Draw white text
	display.cp437(true);					// Use full 256 char 'Code Page 437' font
	
	display.setCursor(32,0);
	display.write("Rocket");
	display.setCursor(40,16);
	display.write("Track");
	
	
	char buffer[32];
	sprintf(buffer,"ID:  %03d",lora_id);

	display.setCursor(24,40);
	display.write(buffer);
	
	display.display();	
#endif

	return(0);
}

void PollDisplay(void)
{
	if(!display_enable)
		return;
	
	static int DisplayState=0;
	static int LastDisplayChange=0;
	
	if(millis()<display_update_suspend)
		return;
	
	if(millis()>=(LastDisplayChange+DISPLAY_UPDATE_PERIOD))
	{
#ifdef ARDUINO_HELTEC_WIRELESS_TRACKER


#else
		display.clearDisplay();
		
		// portrait
		display.setRotation(1);
		
//		display.setTextSize(1);
		
		// draw white on black if logging is active, inverted otherwise
		if(sdcard_enable)	display.setTextColor(SSD1306_WHITE);
		else				display.setTextColor(SSD1306_INVERSE);
		
		display.setCursor(0,0);
		
		char buffer[32];
		
		display.setTextSize(1);
		
		sprintf(buffer,"%04d/%02d/%02d\r\n",beaconyear,beaconmonth,beaconday);
		display.print(buffer);
		
		sprintf(buffer,"  %02d%02d%02d\r\n",beaconhour,beaconmin,beaconsec);
		display.print(buffer);
		
		switch(DisplayState)
		{
			case 0 ... 5:	display.setTextSize(1);
							display.println();
							display.printf("Lat:\r\n %.6f\r\n",lastfix.latitude);
							display.printf("Lon:\r\n %.6f\r\n",lastfix.longitude);
//							display.printf("Altitude:\r\n %.1f m\r\n",lastfix.height);
							break;
			
			case 6 ... 7:	display.setTextSize(1);
							display.print("\r\n# Sats:\r\n  ");
							display.setTextSize(2);
							display.println(lastfix.numsats);
							break;

#if 0
			case 8 ... 11:	display.setTextSize(1);
							display.print("\r\nGPS Alt\r\nCurr\r\n");
							display.setTextSize(2);
							display.printf("%.1f\r\n",lastfix.height/1e3);
							break;
#endif
						
			case 8 ... 11:	display.setTextSize(1);
							display.print("\r\nBaro Alt\r\nCurr\r\n");
							display.setTextSize(2);
							display.printf("%.1f\r\n",lastfix.height);
							display.setTextSize(1);
							display.print("Max\r\n");
							display.setTextSize(2);
							display.printf("%.1f\r\n",max_baro_height);
							break;
			
			case 12 ... 16:	display.setTextSize(2);
							display.print("\r\nLoRa\r\n  ID:\r\n\n");
//							display.setTextSize(3);
							display.printf("  %03d",lora_id);
							break;
		
			default:		
							DisplayState=0;
							break;
		}
		
		display.setTextSize(3);
		display.setCursor(0,104);
		if(lastfix.gpsfix==3)		display.println("3D");
		else if(lastfix.gpsfix==2)	display.println("2D");
		else						display.println("NF");
			
		display.display();

		SetTXIndicator(tx_active);
		
#endif
		DisplayState++;
		if(DisplayState>=16)
			DisplayState=0;
		
		LastDisplayChange=millis();
	}
}

void SetTXIndicator(int on)
{
	if(!display_enable)
		return;
	
	if(on)
	{
#ifdef ARDUINO_HELTEC_WIRELESS_TRACKER
#else
		display.setTextSize(2);
		display.setCursor(48,128-32);
		display.print("T");
		display.setCursor(48,128-16);
		display.print("X");
		display.display();
#endif
	}
	else
	{
#ifdef ARDUINO_HELTEC_WIRELESS_TRACKER
#else
		display.setTextSize(2);
		display.setCursor(48,128-32);
		display.print(" ");
		display.setCursor(48,128-16);
		display.print(" ");
		display.display();
#endif
	}
	
}

void ShowModeChange(void)
{
	if(!display_enable)
		return;
	
#if ARDUINO_HELTEC_WIRELESS_TRACKER
#else
	display.clearDisplay();

	display.setTextSize(4);
	display.setCursor(8,48);
	
	if(strcmp(lora_mode,"High Rate")==0)	{	display.print("HR");	}
	else									{	display.print("LR");	}

	display.setTextSize(2);
	display.setCursor(8,85);
	display.print("Mode");
	
	display.display();
#endif

	display_update_suspend=millis()+3000;	
}

