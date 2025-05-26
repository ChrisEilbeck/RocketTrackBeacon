
#include "Packetisation.h"
#include "Webserver.h"
#include "WiFiSupport.h"

#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

int webserver_enable=1;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with LED state value
String processor(const String& var)
{
//	Serial.println("webserver process entry");
	
	char buffer[256];
	memset(buffer,0,sizeof(buffer));
	
	if(var=="LORA_MODE")
	{
		sprintf(buffer,lora_mode);
	}
	else if(var=="BAT_STATUS")
	{
		if(axp.isChargeingEnable())
			sprintf(buffer,"\"Charging\"");
		else
			sprintf(buffer,"\"Discharging\"");
	}
	else if(var=="BAT_CURRENT")
	{
/*
		if(axp.isChargeingEnable())
			sprintf(buffer,"%.1f",axp.getBattChargeCurrent());
		else
			sprintf(buffer,"%.1f",axp.getBattDischargeCurrent());
*/
		sprintf(buffer,"%.1f",axp.getBattChargeCurrent()/1e3);
	}
	else if(var=="BAT_VOLTAGE")
	{
		sprintf(buffer,"%.3f",axp.getBattVoltage()/1000);
	}
	else if(var=="LATITUDE")
	{
		if(lastfix.latitude>0)
			sprintf(buffer,"%2.6f N",lastfix.latitude/1e7);
		else
			sprintf(buffer,"%2.6f S",-lastfix.latitude/1e7);
	}
	else if(var=="LONGITUDE")
	{
		if(lastfix.longitude>0)	
			sprintf(buffer,"%3.6f E",lastfix.longitude/1e7);
		else
			sprintf(buffer,"%3.6f W",-lastfix.longitude/1e7);
	}
	else if(var=="ALTITUDE")
	{
		sprintf(buffer,"%.1f",lastfix.height/1e3);
	}
	else if(var=="NUM_CHANNELS")
	{
		sprintf(buffer,"%d",lastfix.numsats);
	}
	else if(var=="GPS_FIX")
	{
		sprintf(buffer,"%d",gpsFix);
	}
	else if(var=="HORIZONTAL_ACCURACY")
	{
		sprintf(buffer,"%.1f",lastfix.accuracy);
	}
	else if(var=="SAT_NUMS")
	{
		int cnt=0;
		for(cnt=0;cnt<lastfix.numsats;cnt++)
		{
			if(cnt!=(lastfix.numsats-1))
				sprintf(buffer+strlen(buffer),"%d,",svid[cnt]);
			else
				sprintf(buffer+strlen(buffer),"%d",svid[cnt]);
		}
	}
	else if(var=="SAT_ELEVS")
	{
		int cnt=0;
		for(cnt=0;cnt<lastfix.numsats;cnt++)
		{
			if(cnt!=(lastfix.numsats-1))
				sprintf(buffer+strlen(buffer),"%d,",elev[cnt]);
			else
				sprintf(buffer+strlen(buffer),"%d",elev[cnt]);
		}
	}
	else if(var=="SAT_AZS")
	{
		int cnt=0;
		for(cnt=0;cnt<lastfix.numsats;cnt++)
		{
			if(cnt!=(lastfix.numsats-1))
				sprintf(buffer+strlen(buffer),"%d,",azim[cnt]);
			else
				sprintf(buffer+strlen(buffer),"%d",azim[cnt]);
		}
	}
	else if(var=="SAT_SNRS")
	{
		int cnt=0;
		for(cnt=0;cnt<lastfix.numsats;cnt++)
		{
			if(cnt!=(lastfix.numsats-1))
				sprintf(buffer+strlen(buffer),"%d,",cno[cnt]);
			else
				sprintf(buffer+strlen(buffer),"%d",cno[cnt]);
		}
	}
	
//	Serial.println("webserver process exit");
	
	if(strlen(buffer)>0)
		return(buffer);
	else
		return String();
}

int SetupWebServer(void)
{
	if(		wifi_enable
		&&	spiffs_enable
		&&	webserver_enable	)
	{
		// Route for root / web page
		server.on("/",HTTP_GET,[](AsyncWebServerRequest *request)					{														request->redirect("/status.html");		});	
		
		server.on("/status.html",HTTP_GET,[](AsyncWebServerRequest *request)		{	Serial.println("Returning /status.html");			request->send(SPIFFS,"/status.html");	});	
		server.on("/status.css",HTTP_GET,[](AsyncWebServerRequest *request)			{	Serial.println("Returning /status.css");			request->send(SPIFFS,"/status.css");	});	
		
		server.on("/status.js",HTTP_GET,[](AsyncWebServerRequest *request)
		{
			Serial.println("Returning modified /status.js");
			request->send(SPIFFS,"/status.js",String(),false,processor);
		});	
		
		server.on("/config.html",HTTP_GET,[](AsyncWebServerRequest *request)		{	Serial.println("Returning /config.html");			request->send(SPIFFS,"/config.html");	});	
		server.on("/config.css",HTTP_GET,[](AsyncWebServerRequest *request)			{	Serial.println("Returning /config.css");			request->send(SPIFFS,"/config.css");	});	
		server.on("/config.js",HTTP_GET,[](AsyncWebServerRequest *request)			{	Serial.println("Returning /config.js");				request->send(SPIFFS,"/config.js");		});	
		
		server.on("/engineering.html",HTTP_GET,[](AsyncWebServerRequest *request)	{	Serial.println("Returning /engineering.html");		request->send(SPIFFS,"/engineering.html");		});	
		server.on("/engineering.css",HTTP_GET,[](AsyncWebServerRequest *request)	{	Serial.println("Returning /engineering.css");		request->send(SPIFFS,"/engineering.css");		});	
		server.on("/engineering.js",HTTP_GET,[](AsyncWebServerRequest *request)		{	Serial.println("Returning /engineering.js");		request->send(SPIFFS,"/engineering.js");		});	
		
		server.on("/pton.html",HTTP_POST,[](AsyncWebServerRequest *request)
		{
			Serial.println("Turning permanent transmit ON");
			lora_constant_transmit=true;
			request->redirect("/engineering.html");
		});	
		
		server.on("/ptoff.html",HTTP_POST,[](AsyncWebServerRequest *request)
		{
			Serial.println("Turning permanent transmit off");
			lora_constant_transmit=false;
			request->redirect("/engineering.html");
		});	
		
		server.on("/longrange.html",HTTP_POST,[](AsyncWebServerRequest *request)
		{
			Serial.println("Setting to Long Range mode");
			strcpy(lora_mode,"Long Range");
			SetLoRaMode(lora_mode);
			request->redirect("/engineering.html");
		});	
		
		server.on("/highrate.html",HTTP_POST,[](AsyncWebServerRequest *request)
		{
			Serial.println("Setting to High Rate mode");
			strcpy(lora_mode,"High Rate");
			SetLoRaMode(lora_mode);
			request->redirect("/engineering.html");
		});	
		
		// Start server
		server.begin();
		
		return(0);
	}
	else
	{
		return(1);	
	}
}

void PollWebServer(void)
{
	
}

