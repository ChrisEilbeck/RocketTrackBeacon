
#include "Display.h"
#include "Packetisation.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3c ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define DISPLAY_UPDATE_PERIOD	1000

Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire,OLED_RESET);

int display_update_suspend=0;

int SetupDisplay(void)
{
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if(!display.begin(SSD1306_SWITCHCAPVCC,SCREEN_ADDRESS))
	{
		Serial.println(F("SSD1306 allocation failed"));
		return(1);
	}
	
	Serial.println(F("SSD1306 display configured ..."));
	
	display.setRotation(2);
	
	display.clearDisplay();
	
	display.setTextSize(2);					// Normal 1:1 pixel scale
	display.setTextColor(SSD1306_WHITE);	// Draw white text
	display.cp437(true);					// Use full 256 char 'Code Page 437' font
	
	display.setCursor(32,16);
	display.write("Rocket");
	display.setCursor(40,32);
	display.write("Track");
	
	display.display();
	
	return(0);
}

void PollDisplay(void)
{
	static int DisplayState=0;
	static int LastDisplayChange=0;
	
	if(millis()<display_update_suspend)
		return;
	
	if(millis()>=(LastDisplayChange+DISPLAY_UPDATE_PERIOD))
	{
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
							display.printf("Lat:\r\n %.6f\r\n",lastfix.latitude/1e7);
							display.printf("Lon:\r\n %.6f\r\n",lastfix.longitude/1e7);		
							display.printf("Altitude:\r\n %.1f m\r\n",lastfix.height/1e3);							
							break;
			
			case 6 ... 7:	display.setTextSize(1);
							display.print("\r\n# Sats:\r\n  ");
							display.setTextSize(2);
							display.println(beaconnumSats);
							break;

			case 8 ... 11:	display.setTextSize(1);
							display.print("\r\nGPS Alt\r\nCurr\r\n");
							display.setTextSize(2);
							display.printf("%.1f\r\n",lastfix.height/1e3);
							break;
							
			case 12 ... 15:	display.setTextSize(1);
							display.print("\r\nBaro Alt\r\nCurr\r\n");
							display.setTextSize(2);
							display.printf("%.1f\r\n",baro_height);
							display.setTextSize(1);
							display.print("Max\r\n");
							display.setTextSize(2);
							display.printf("%.1f\r\n",max_baro_height);
							break;
		
			default:		
							DisplayState=0;
							break;
		}
		
		display.setTextSize(3);
		display.setCursor(0,104);
		if(gpsFix==3)		display.println("3D");
		else if(gpsFix==2)	display.println("2D");
		else				display.println("NF");
			
		display.display();

		SetTXIndicator(tx_active);
		
		DisplayState++;
		if(DisplayState>=16)
			DisplayState=0;
		
		LastDisplayChange=millis();
	}
}

void SetTXIndicator(int on)
{
	if(on)
	{
		display.setTextSize(2);
		display.setCursor(48,128-32);
		display.print("T");
		display.setCursor(48,128-16);
		display.print("X");
	}
	else
	{
		display.setTextSize(2);
		display.setCursor(48,128-32);
		display.print(" ");
		display.setCursor(48,128-16);
		display.print(" ");
	}
	
	display.display();
}

void ShowModeChange(void)
{
	display.clearDisplay();

	display.setTextSize(4);
	display.setCursor(8,48);
	
	if(strcmp(lora_mode,"High Rate")==0)	{	display.print("HR");	}
	else									{	display.print("LR");	}

	display.setTextSize(2);
	display.setCursor(8,85);
	display.print("Mode");
	
	display.display();

	display_update_suspend=millis()+3000;	
}

