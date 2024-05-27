
#define STATIONMODE 1

#include "WiFiSupport.h"

#include "WiFi.h"
#include <ESPmDNS.h>	

char ssid[32]="RocketTrack";
char password[64]="marsflightcrew";

int wifi_enable=1;

int SetupWiFi(void)
{
	// Act as an Access Point
	
	Serial.print("Setting up WiFi AP with ssid \"");		Serial.print(ssid);	Serial.print("\", password \"");	Serial.print(password);	Serial.println("\"");
	
	// Remove the password parameter, if you want the AP (Access Point) to be open
	WiFi.softAP(ssid,password);
	
	IPAddress IP=WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(IP);
	
	if(!MDNS.begin(ssid))
	{
		Serial.println("Error starting mDNS");
		return(1);
	}	
	
	return(0);
}

