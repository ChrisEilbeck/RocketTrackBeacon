
#pragma once

extern int log_level;
extern int logging_enable;

int SetupLogging(void);
int OpenLogFile(void);
int LogMessage(char *logmsg);

// number of buffers to allocate so we can track multiple beacons at the
// same time one one frequency

#define MAX_BEACONS 10

typedef struct
{
    uint8_t id;
    uint8_t gpsfix;
    uint8_t numsats;
    int8_t snr;
    int8_t rssi;
    uint8_t rxmode;
    uint8_t spare1;
    uint8_t spare2;
    uint8_t spare3;
    float longitude;
    float latitude;
    float height;
    float accuracy;
    float voltage;
    float offset;
    uint16_t counter;
    uint32_t millis;
} fix;

