
#pragma once

extern bool neopixels_enable;

int SetupNeopixels(void);
void PollNeopixels(void);

int NeopixelCommandHandler(uint8_t *cmd,uint16_t cmdptr);

