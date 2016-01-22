#ifndef _RELOJ_H_
#define _RELOJ_H_

#include "types.h"

#define LEDS_OUT P1OUT
#define LEDS_DIR P1DIR
#define R_LED 0x01
#define G_LED 0x02
#define B_LED 0x04

extern volatile u8 hour_pm;
extern volatile unsigned int hour;
extern volatile unsigned int min;
extern volatile unsigned int sec;

extern int alm_hour;
extern int alm_min;
extern u8 alm_pm;
extern u8 alm_on;

extern unsigned int Tick_hour;
extern unsigned int Tick_min;
extern unsigned int Tick_sec;

extern int interval;
extern int hdd_tick_count;

void OrderLedCalls(unsigned int x, unsigned int y, unsigned int z);
void RedLED(unsigned int x);
void GreenLED(unsigned int x);
void BlueLED(unsigned int x);

#endif