
#include "FS.h"
//#include "SD.h"
#include "SD_MMC.h"

bool sdcard_enable=false;

int SetupSDCard(void)
{
#if 0
	Serial.print("SD card disabled in this build ...\r\n");
	return(1);
#else
	while(1)
	{
		Serial.print("Initializing SD card...");
		
//		pinMode(SDCARD_NSS,OUTPUT);
//		digitalWrite(SDCARD_NSS,HIGH);

		SD_MMC.setPins(SDMMC_CLK,SDMMC_CMD,SDMMC_D0,SDMMC_D1,SDMMC_D2,SDMMC_D3);
		
//		if(!SD.begin(SDCARD_NSS,SDCARD_MOSI,SDCARD_MISO,SDCARD_CLK))
		if(!SD_MMC.begin())
		{
			Serial.println("\tinitialization failed!");
			sdcard_enable=false;
			return(1);
		}
		
		uint8_t cardType=SD_MMC.cardType();
//		uint8_t cardType=SD.cardType();

		if(cardType==CARD_NONE)
		{
			Serial.println("\tNo SD_MMC card attached");
			return(1);
		}
		
		Serial.print("\tSD_MMC Card Type: ");
		if(cardType==CARD_MMC)			{	Serial.println("MMC");		}
		else if(cardType==CARD_SD)		{	Serial.println("SDSC");		}
		else if(cardType==CARD_SDHC)	{	Serial.println("SDHC");		}
		else							{	Serial.println("UNKNOWN");	}

		uint64_t cardSize=SD_MMC.cardSize()/(1024*1024);
//		uint64_t cardSize=SD.cardSize()/(1024*1024);
		Serial.printf("\tSD_MMC Card Size: %lluMB\n",cardSize);

		Serial.println("initialization done.");	
		
		delay(2000);
	}
	
	return(0);
#endif
}

