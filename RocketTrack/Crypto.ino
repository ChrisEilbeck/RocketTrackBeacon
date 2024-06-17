
#include <Crypto.h>
#include <AES.h>

int crypto_enable=1;

AES128 aes128;
AES192 aes192;
AES256 aes256;

BlockCipher *cipher=&aes256;

uint8_t crypto_key[32];

char crypto_key_hex[65];

void HexToUint8(char *hex,uint8_t *binary)
{
	int cnt;
	for(cnt=0;cnt<strlen(hex);cnt+=2)
	{
		binary[cnt/2]=0;
		
		if(	(hex[cnt]>='0')&&(hex[cnt]<='9')		)	binary[cnt/2]+=16*(hex[cnt]-'0');
		if(	(hex[cnt]>='a')&&(hex[cnt]<='f')		)	binary[cnt/2]+=16*(hex[cnt]-'a'+10);
		if(	(hex[cnt]>='A')&&(hex[cnt]<='F')		)	binary[cnt/2]+=16*(hex[cnt]-'A'+10);
		
		if(	(hex[cnt+1]>='0')&&(hex[cnt+1]<='9')	)	binary[cnt/2]+=hex[cnt+1]-'0';
		if(	(hex[cnt+1]>='a')&&(hex[cnt+1]<='f')	)	binary[cnt/2]+=hex[cnt+1]-'a'+10;
		if(	(hex[cnt+1]>='A')&&(hex[cnt+1]<='F')	)	binary[cnt/2]+=hex[cnt+1]-'A'+10;
	}
}

int SetupCrypto(void)
{
	Serial.print("Configuring AES crypto\r\n");
	
	uint8_t key[32]={	0x60,0x3d,0xeb,0x10,0x15,0xca,0x71,0xbe,0x2b,0x73,0xae,0xf0,0x85,0x7d,0x77,0x81,0x1f,0x35,0x2c,0x07,0x3b,0x61,0x08,0xd7,0x2d,0x98,0x10,0xa3,0x09,0x14,0xdf,0xf4	};
	uint8_t pt[16]={	0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a	};
	uint8_t ct[16];
	uint8_t tv[16]={	0xf3,0xee,0xd1,0xbd,0xb5,0xd2,0xa0,0x3c,0x06,0x4b,0x5a,0x7e,0x3d,0xb1,0x81,0xf8	};
	
//	crypto_feed_watchdog();
	cipher->setKey(key,cipher->keySize());
	cipher->encryptBlock(ct,pt);
	
	int cnt;
	for(cnt=0;cnt<sizeof(tv);cnt++)
	{
		if(ct[cnt]!=tv[cnt])
		{
			Serial.print("\tCrypto check FAILED!  Halting ...\r\n");
			return(1);
		}
	}
	
	Serial.print("\tCrypto checked out OK\r\n");
	
	Serial.println("\tSetting up crypto from config file");
	
	if(strlen(crypto_key_hex)==0)			{	Serial.println("\tWarning: no crypto selected!");		cipher=NULL;		}
	else if(strlen(crypto_key_hex)==32)		{	Serial.println("\tSelected AES128 mode");				cipher=&aes128;		}
	else if(strlen(crypto_key_hex)==48)		{	Serial.println("\tSelected AES192 mode");				cipher=&aes192;		}
	else if(strlen(crypto_key_hex)==64)		{	Serial.println("\tSelected AES256 mode");				cipher=&aes256;		}
	else
	{
		Serial.println("\tInvalid crypto settings detected!");
		return(1);
	}
	
	HexToUint8(crypto_key_hex,crypto_key);
	
	cipher->setKey(crypto_key,cipher->keySize());
	
	return(0);
}

void EncryptPacket(uint8_t *packet)
{
	if(		!crypto_enable
		||	(cipher==NULL)	)
	{
		Serial.println("Warning: crypto disabled");
		return;
	}
	else
	{
		uint8_t pt[16];
		memcpy(pt,packet,16);
		memset(packet,0,16);
//		crypto_feed_watchdog();
		cipher->setKey(crypto_key,cipher->keySize());
		cipher->encryptBlock(packet,pt);
	}
}

void DecryptPacket(uint8_t *packet)
{
	if(		!crypto_enable
		||	(cipher==NULL)	)
	{
		Serial.println("Warning: crypto disabled");
		return;
	}
	else
	{
		uint8_t ct[16];
		memcpy(ct,packet,16);
		memset(packet,0,16);
//		crypto_feed_watchdog();
		cipher->setKey(crypto_key,cipher->keySize());
		cipher->decryptBlock(packet,ct);
	}
}

