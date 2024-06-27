
#ifndef ADAFRUIT_FEATHER_M0

#include "Logging.h"
#include "LoRaModule.h"
#include "Packetisation.h"
#include "Webserver.h"
#include "WiFiSupport.h"

#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

int webserver_enable=1;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String index_processor(const String& var)
{
#if 0
	Serial.print("Logging: ");	Serial.println(logging_enable?"ACTIVE":"off");
	Serial.print("Transmit: ");	Serial.println(lora_constant_transmit?"ACTIVE":"off");
#endif
		
	char buffer[256];
	memset(buffer,0,sizeof(buffer));
	
	if(var=="LOGGING_ENABLED")
	{
		if(logging_enable)			sprintf(buffer,"ACTIVE");
		else						sprintf(buffer,"off");
	}
	else if(var=="CONSTANT_TRANSMIT")
	{
		if(lora_constant_transmit)	sprintf(buffer,"ACTIVE");
		else						sprintf(buffer,"off");
	}
	
	if(strlen(buffer)>0)			return(buffer);
	else							return String();
}

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
#if 0
	else if(var=="LATITUDE")
	{
		if(beaconlat>0)	sprintf(buffer,"%2.6f N",beaconlat/1e7);	else	sprintf(buffer,"%2.6f S",-beaconlat/1e7);
	}
	else if(var=="LONGITUDE")
	{
		if(beaconlon>0)	sprintf(buffer,"%3.6f E",beaconlon/1e7);	else	sprintf(buffer,"%3.6f W",-beaconlon/1e7);
	}
	else if(var=="ALTITUDE")
	{
		sprintf(buffer,"%.1f",beaconheight/1e3);
	}
#endif
#if 0
	else if(var=="NUM_CHANNELS")
	{
		sprintf(buffer,"%d",beaconnumsats);
	}
	else if(var=="GPS_FIX")
	{
		sprintf(buffer,"%d",gpsFix);
	}
	else if(var=="HORIZONTAL_ACCURACY")
	{
		sprintf(buffer,"%.1f",beaconhacc);
	}
	else if(var=="SAT_NUMS")
	{
		int cnt=0;
		for(cnt=0;cnt<beaconnumsats;cnt++)
		{
			if(cnt!=(beaconnumsats-1))
				sprintf(buffer+strlen(buffer),"%d,",svid[cnt]);
			else
				sprintf(buffer+strlen(buffer),"%d",svid[cnt]);
		}
	}
	else if(var=="SAT_ELEVS")
	{
		int cnt=0;
		for(cnt=0;cnt<beaconnumsats;cnt++)
		{
			if(cnt!=(beaconnumsats-1))
				sprintf(buffer+strlen(buffer),"%d,",elev[cnt]);
			else
				sprintf(buffer+strlen(buffer),"%d",elev[cnt]);
		}
	}
	else if(var=="SAT_AZS")
	{
		int cnt=0;
		for(cnt=0;cnt<beaconnumsats;cnt++)
		{
			if(cnt!=(beaconnumsats-1))
				sprintf(buffer+strlen(buffer),"%d,",azim[cnt]);
			else
				sprintf(buffer+strlen(buffer),"%d",azim[cnt]);
		}
	}
	else if(var=="SAT_SNRS")
	{
		int cnt=0;
		for(cnt=0;cnt<beaconnumsats;cnt++)
		{
			if(cnt!=(beaconnumsats-1))
				sprintf(buffer+strlen(buffer),"%d,",cno[cnt]);
			else
				sprintf(buffer+strlen(buffer),"%d",cno[cnt]);
		}
	}
#endif
	
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
		// catch anything we're asked for
		server.onNotFound([](AsyncWebServerRequest *request)						{   request->redirect("/");						});
		
		// Route for root / web page
		server.on("/",HTTP_GET,[](AsyncWebServerRequest *request)					{	request->redirect("/index.html");			});

		server.on("/index.html",HTTP_GET,[](AsyncWebServerRequest *request)			{	request->send(SPIFFS,"/index.html",
																										String(),false,
																										index_processor);			});
		
		server.on("/index.css",HTTP_GET,[](AsyncWebServerRequest *request)			{	request->send(SPIFFS,"/index.css");			});
		server.on("/overall.css",HTTP_GET,[](AsyncWebServerRequest *request)		{	request->send(SPIFFS,"/overall.css");		});
		server.on("/logo.jpg",HTTP_GET,[](AsyncWebServerRequest *request)			{	request->send(SPIFFS,"/logo.jpg");			});
		server.on("/favicon.ico",HTTP_GET,[](AsyncWebServerRequest *request)		{	request->send(SPIFFS,"/favicon.ico");		});
		server.on("/includehtml.js",HTTP_GET,[](AsyncWebServerRequest *request)		{	request->send(SPIFFS,"/includehtml.js");	});
		server.on("/buttons.html",HTTP_GET,[](AsyncWebServerRequest *request)		{	request->send(SPIFFS,"/buttons.html");		});

		server.on("/log_on.html",HTTP_GET,[](AsyncWebServerRequest *request)		{	logging_enable=true;
																						Serial.println("Setting logging ACTIVE");
																						request->redirect("/index.html");			});
		server.on("/log_off.html",HTTP_GET,[](AsyncWebServerRequest *request)		{	logging_enable=false;
																						Serial.println("Setting logging off");
																						request->redirect("/index.html");			});
		server.on("/tx_on.html",HTTP_GET,[](AsyncWebServerRequest *request)			{	lora_constant_transmit=true;
																						Serial.println("Setting transmit ACTIVE");
																						request->redirect("/index.html");			});
		server.on("/tx_off.html",HTTP_GET,[](AsyncWebServerRequest *request)		{	lora_constant_transmit=false;
																						Serial.println("Setting transmit off");
																						request->redirect("/index.html");			});

#if 0		
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
#endif
				
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

#endif

