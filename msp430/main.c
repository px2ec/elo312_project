#include "msp430_version.h"            // Depende del uC que Ud. estÃ© ocupando.
#include "uart.h"
#include "osc.h"
#include "types.h"
#include "spi.h"
#include "sd.h"
#include "tracknfo.h"
#include "reloj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NTP_SYNC	2
#define SET_ALARM 	4

#ifndef NULL
#define NULL ((void *) 0)
#endif

// SD variasbles and defines
#define PERIPH_CLOCKRATE 8000000
int do_sd_initialize(sd_context_t *sdc);

static sd_context_t sdc;
static u8 sd_buffer[512];

static u32 start_block = TRACK0_STA_BLK;
volatile u8 get_next = 0;
volatile u8 snd_play = 0;
volatile static u8 to_dac[512];

// UART variables
int recv0_char = 0x00;
unsigned char recv0_flag = 0x00;
int recv1_char = 0x00;
unsigned char recv1_flag = 0x00;

// Comm
typedef struct minipkt {
	int size; // bytes of parameters
	unsigned char param[99]; // array of parameters
	char intr ; // instruction
	int ready; // packet status
	} packet;

packet txpkt;
packet buffpkt; // global
unsigned char enflag = 0; // enable glaf for packet
char lastch = '\0'; // last character

void reset_packet(packet* arg) {
	arg->ready = 0;
	arg->intr = '\0';
	arg->size = 0;
}

void flush_rxpkt() {
	for (int i = 0; i < 99; i++)
		buffpkt.param[i] = '\0';
	reset_packet(&buffpkt);
	enflag = 0; 
}

/*
void flush_txpkt() {
	for (int i = 0; i < 99; i++)
		txpkt.param[i] = '\0';
	reset_packet(&txpkt);
	txpkt.intr = 'R';
}

void send_packet(packet pkt) {
	uart0_send(pkt.intr);
	uart0_send(':');
	for (int i = 0; i < pkt.size; i++)
		uart0_send(pkt.param[i]);
}*/

// Main ----------------------------------
int main(void) {
	u16 i;
	u32 j;
	osc_init_xt2();

	WDTCTL = WDT_ADLY_1000;

	uart1_init_p1();
	flush_rxpkt();

	/* DAC12: Initialization */
	ADC12CTL0 = REF2_5V | REFON;
	for (i = 0xFFFF; i > 0; i--)
		asm("nop");
	DAC12_0CTL = DAC12AMP_7 | DAC12DF | DAC12IR | DAC12LSEL_1;
	DAC12_0CTL |= DAC12CALON;
	P6DIR = BIT6;
	P6SEL = BIT6;
	/* DAC12: Wait for calibration */
	while (DAC12_0CTL & DAC12CALON);

	/* Timer A */
	TACTL = TASSEL_2 | MC_1;
	TACCTL0 = CCIE;
	TACCR0 = PERIPH_CLOCKRATE / 11030;

	/* SD code below */
	int ok = 0;

	/* Set some reasonable values for the timeouts */
	sdc.timeout_write = PERIPH_CLOCKRATE;
	sdc.timeout_read = PERIPH_CLOCKRATE;
	sdc.busyflag = 0;

	for (i = 0; i < 1000; i++) {
		__delay_cycles(16000);
	}

	for (i = 0; i < SD_INIT_TRY && ok != 1; i++) {
		ok = do_sd_initialize(&sdc);
	}

	if (ok == 0)
		return -1;


	/* Clock */
	IE1 |= WDTIE;
	LEDS_DIR = R_LED | G_LED | B_LED;
	LEDS_OUT = R_LED | G_LED | B_LED;

	TBCCTL0 = CCIE;
	TBCTL = TBSSEL_2 + ID_3;

	/* setup (measure period) */
	while (P2IN == 0x00);
	while (P2IN == 0x01);
	TBCTL |= MC_2;
	while (P2IN == 0x00);
	while (P2IN == 0x01);
	hdd_tick_count = TBR;
	TBCTL &= ~(3 * 0x10u);
	TBCTL |= TBCLR;
	
	interval = hdd_tick_count / 60;


	__bis_SR_register(GIE);

	/* Wait forever */
	i = 0;
	j = 0;
	while (1) {
		/* Read in the first block on the SD card */
		if (get_next == 1) {
			spi_enable();
			get_next = 0;

			sd_read_block(&sdc, start_block + i, sd_buffer);
			sd_wait_notbusy(&sdc);

			i++;
			if (i >= MAX_NUM_BLOCKS) {
				i = 0;
				snd_play = 0;
			}
			spi_disable();
		}

		if (alm_on == 1 && hour == alm_hour && min == alm_min && hour_pm == alm_pm) {
			alm_on = 0;
			snd_play = 1;
		}

		if (!snd_play) {
			while (P2IN == 0x01);
			Tick_hour = hour * 5 * interval;
			Tick_min = min * interval;
			Tick_sec = sec * interval;
			OrderLedCalls(Tick_hour, Tick_min, Tick_sec);
			while (P2IN == 0x00);
			j = 0;
		} else {
			if (j < (10000)) {
				LEDS_OUT = R_LED;
			} else if (j < (20000)) {
				LEDS_OUT = G_LED;
			} else if (j < (30000)) {
				LEDS_OUT = B_LED;
			} else {
				j = 0;
			}

			j++;
		}
	}

	return 0;
}

// UART ----------------------------------

void options() {
	if (buffpkt.ready != 1) return;
	switch (buffpkt.intr) {
	case NTP_SYNC: 
		// fill with setup rtc method
		// printf("hour: %d \t minute: \t %d second: %d\n", (int)buffpkt.param[0], (int)buffpkt.param[1], (int)buffpkt.param[2]); // eliminate when i2c and sd are implemented
		hour = (unsigned int)buffpkt.param[0];
		min = (unsigned int)buffpkt.param[1];
		sec = (unsigned int)buffpkt.param[2];

		hour_pm = (hour >= 12) ? 1 : 0;
		hour = hour % 12;
		break;
	case SET_ALARM:
		// fill with alarm variables and method
		// printf("hour: %d \t minute: %d\n", (int)buffpkt.param[0], (int)buffpkt.param[1]); // eliminate when i2c and sd are implemented
		if ((int)buffpkt.param[2] == 1) {
			start_block = TRACK0_STA_BLK;
		} else if ((int)buffpkt.param[2] == 2) {
			start_block = TRACK1_STA_BLK;
		} else if ((int)buffpkt.param[2] == 3) {
			start_block = TRACK2_STA_BLK;
		} else if ((int)buffpkt.param[2] == 4) {
			start_block = TRACK3_STA_BLK;
		} else {
			start_block = TRACK4_STA_BLK;
		}

		if (alm_min != min || alm_hour != hour || alm_pm != hour_pm) {
			alm_hour = (int)buffpkt.param[0];
			alm_min = (int)buffpkt.param[1];

			alm_pm = (alm_hour >= 12) ? 1 : 0;
			alm_hour = alm_hour % 12;

			alm_on = 1;
		}

		break;
	}
	flush_rxpkt();
}

void *recv1_callback(void *ptr) {
	char input = *(char *)ptr;
	//uart0_send(input);
	//printf("%d \n", (int)input);
	if (lastch == 255 && input == 255 && enflag == 0) {
		enflag = 1;
	} else if (input != 255 && lastch == 255 && buffpkt.intr == '\0' && enflag) {
		buffpkt.intr = input;
	} else if (enflag == 1 && buffpkt.size >= 0) {
		buffpkt.param[buffpkt.size++] = 255 - input;
		if (input == '\r') {
			buffpkt.ready = 1;
			options();
		}
	}
	lastch = input;
	return NULL;
}
void *(*uart1_int_recv_callback)(void *) = &recv1_callback;

void *recv0_callback(void *ptr) {
	char input = *(char *)ptr;
	//uart0_send(input);
	return NULL;
}
void *(*uart0_int_recv_callback)(void *) = &recv0_callback;

// SD ------------------------------------

int do_sd_initialize(sd_context_t *sdc)
{
	/* Initialize the SPI controller */
	spi_initialize();

	/* Start out with a slow SPI clock, 400kHz, as requred by the SD spec */
	spi_set_divisor(PERIPH_CLOCKRATE/400000);

	/* Initialization OK? */
	if (sd_initialize(sdc) != 1)
		return 0;

	/* Set the SPI clock rate */
	spi_set_divisor(16);

	return 1;
}

#pragma vector = TIMERA0_VECTOR
__interrupt void isr_timer_a(void)
{
	__bic_SR_register(GIE);
	if (snd_play == 0)
		goto _abort_TA;

	static u16 i = 0;
	DAC12_0DAT = 0x0FFF & (((sd_buffer[i + 1] << 8) | sd_buffer[i]) >> 4);
	DAC12_0CTL |= DAC12ENC;

	if (i < SD_BLOCKSIZE) {
		i = i + 2;
	} else {
		i = 0;
		get_next = 1;
	}

_abort_TA:
	__bis_SR_register_on_exit(GIE);
	return;
}