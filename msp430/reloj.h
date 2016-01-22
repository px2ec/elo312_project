#ifndef _RELOJ_H_
#define _RELOJ_H_

#include "types.h"

#define LEDS_OUT P1OUT
#define LEDS_DIR P1DIR
#define R_LED 0x01
#define G_LED 0x02
#define B_LED 0x04

extern volatile u8 hour_pm;
extern volatile int hour;
extern volatile int min;
extern volatile int sec;

extern int alm_hour;
extern int alm_min;
extern u8 alm_pm;
extern u8 alm_on;

extern int Tick_hour;
extern int Tick_min;
extern int Tick_sec;

extern int interval;
extern int hdd_tick_count;

void OrderLedCalls(int x, int y, int z);
void RedLED(int x);
void GreenLED(int x);
void BlueLED(int x);

#endif