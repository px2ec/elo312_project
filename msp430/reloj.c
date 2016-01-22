#include <msp430.h>
#include "reloj.h"
#include "types.h"

volatile u8 hour_pm = 0;
volatile int hour = 0;
volatile int min = 30;
volatile int sec = 52;

int alm_hour = -1;
int alm_min = -1;
u8 alm_pm = 0;
u8 alm_on = 0;

int count = 0;
int state = 0;
int wait_flag = 0;
int first_pass = 0;
int hdd_tick_count = 0;
int interval = 0;

int Tick_hour = 1;
int Tick_min = 0;
int Tick_sec = 0;

int flag = 0;
/*
void OrderLedCalls(int tick_hour, int y, int z)	// Since my hdd spins counter-clockwise I picked the biggest value as I go back to the begining of the cicle
{				// if your clock spins clockwise revert the order to work from min->max ( x=hours , y=mins , z=secs )
	if (tick_hour <= y && tick_hour <= z) {
		if (y < z) {
			z = hdd_tick_count - z;
			y = hdd_tick_count - (y + z);
			tick_hour = hdd_tick_count - (tick_hour + y + z);
			GreenLED(tick_hour);
			RedLED(y);
			BlueLED(z);	//max=z
			//mid=y
		} else {
			y = hdd_tick_count - y;
			z = hdd_tick_count - (y + z);
			tick_hour = hdd_tick_count - (tick_hour + y + z);
			RedLED(tick_hour);
			GreenLED(z);
			BlueLED(y);	//max=y
		}
		//min=x
	}

	else if (y <= tick_hour && y <= z) {
		if (tick_hour < z) {
			z = hdd_tick_count - z;
			tick_hour = hdd_tick_count - (tick_hour + z);
			y = hdd_tick_count - (tick_hour + y + z);
			BlueLED(y);
			RedLED(tick_hour);
			GreenLED(z);
		} else {
			tick_hour = hdd_tick_count - tick_hour;
			z = hdd_tick_count - (tick_hour + z);
			y = hdd_tick_count - (tick_hour + y + z);
			BlueLED(y);
			RedLED(tick_hour);
			GreenLED(z);
		}
	}

	else if (z <= tick_hour && z <= y) {
		if (tick_hour < y) {
			y = hdd_tick_count - y;
			tick_hour = hdd_tick_count - (y + tick_hour);
			z = hdd_tick_count - (tick_hour + y + z);
			GreenLED(y);
			RedLED(tick_hour);
			BlueLED(z);
		} else {
			tick_hour = hdd_tick_count - tick_hour;
			y = hdd_tick_count - (y + tick_hour);
			z = hdd_tick_count - (tick_hour + y + z);
			GreenLED(z);
			BlueLED(y);
			RedLED(tick_hour);
		}

	}
}
*/

void OrderLedCalls(int x, int y, int z)
{
	RedLED(x);
	while(P2IN==0x01);
	GreenLED(y);
	while(P2IN==0x01);
	BlueLED(z);
       	while(P2IN==0x01);
}

void RedLED(int x)
{
	wait_flag = 1;
	TBCCR0 = (x == 0) ? 1 : x;
	TBCTL |= MC_1;
	while (wait_flag);
	LEDS_OUT |= R_LED;
	for (int i = 0; i < 150; i++) {
		asm("nop");
	}

	LEDS_OUT &= ~R_LED;
}

void GreenLED(int x)
{
	wait_flag = 1;
	TBCCR0 = (x == 0) ? 1 : x;
	TBCTL |= MC_1;
	while (wait_flag);
	LEDS_OUT |= G_LED;
	for (int i = 0; i < 150; i++) {
		asm("nop");
	}

	LEDS_OUT &= ~G_LED;
}

void BlueLED(int x)
{
	wait_flag = 1;
	TBCCR0 = (x == 0) ? 1 : x;
	TBCTL |= MC_1;
	while (wait_flag);
	LEDS_OUT |= B_LED;
	for (int i = 0; i < 150; i++) {
		asm("nop");
	}

	LEDS_OUT &= ~B_LED;
}

#pragma vector = TIMERB0_VECTOR
__interrupt void isr_timer_b(void)
{
	wait_flag = 0;
	TBCTL &= ~(3 * 0x10u);	// MC_0, stop timer
	TBCTL |= TBCLR;		// Reset TBR, ID_x and count direction
}

#pragma vector = WDT_VECTOR
__interrupt void isr_timer_wd(void)
{
	if ((min == 59) && (sec == 59)) {
		if (hour == 11) {
			hour = 0;
			hour_pm = (hour_pm == 0) ? 1 : 0;
		}

		else
			hour++;
	}

	if (sec == 59) {
		if (min == 59)
			min = 0;
		else
			min++;
		sec = 0;
	} else {
		sec++;
	}
}