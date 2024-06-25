
#ifndef ADAFRUIT_FEATHER_M0
	#include "FS.h"
//	#include "SD.h"
	#include "SD_MMC.h"
#else

#endif

bool sdcard_enable=false;

int SetupSDCard(void)
{
#if 0
	Serial.print("SD card disabled in this build ...\r\n");
	return(1);
#else
	Serial.print("Initializing SD card...");
	
	pinMode(SDCARD_NSS,OUTPUT);
	digitalWrite(SDCARD_NSS,HIGH);

#if 1
	SPIClass sd_spi(HSPI);
	sd_spi.begin(SDCARD_SCK,SDCARD_MISO,SDCARD_MOSI,SDCARD_NSS);

	if (!SD.begin(SDCARD_NSS,sd_spi))
	{
    	Serial.println("SD Card: mounting failed.");
    	return(1);
	}

#else
	SPIClass spiSD=SPIClass(HSPI);
	spiSD.begin();

//	SD_MMC.setPins(SDMMC_CLK,SDMMC_CMD,SDMMC_D0,SDMMC_D1,SDMMC_D2,SDMMC_D3);
//	if(!SD.begin(SDCARD_NSS,SDCARD_MOSI,SDCARD_MISO,SDCARD_CLK))
//	if(!SD.begin(SDCARD_NSS,spiSD))
	if(!SD_MMC.begin(spiSD))
	{
		Serial.println("\tinitialization failed!");
		sdcard_enable=false;
		return(1);
	}
#endif
	
	uint8_t cardType=SD.cardType();
//	uint8_t cardType=SD_MMC.cardType();

	if(cardType==CARD_NONE)
	{
		Serial.println("\tNo SD_MMC card attached");
		return(1);
	}
		
	Serial.print("\tCard Type: ");
	if(cardType==CARD_MMC)			{	Serial.println("MMC");		}
	else if(cardType==CARD_SD)		{	Serial.println("SDSC");		}
	else if(cardType==CARD_SDHC)	{	Serial.println("SDHC");		}
	else							{	Serial.println("UNKNOWN");	}

	uint64_t cardSize=SD.cardSize()/(1024*1024);
//	uint64_t cardSize=SD_MMC.cardSize()/(1024*1024);
	Serial.printf("\tCard Size: %lluMB\n",cardSize);

	Serial.println("initialization done.");
		
	return(0);
#endif
}

