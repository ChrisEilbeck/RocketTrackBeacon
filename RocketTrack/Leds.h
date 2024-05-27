
#pragma once

extern int leds_enable;

int SetupLEDs(void);

void PollLEDs(void);

void led_control(uint32_t led_pattern,uint16_t led_repeat_count);

