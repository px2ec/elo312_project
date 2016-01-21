#include "msp430_version.h"            // Depende del uC que Ud. estÃ© ocupando.
#include "uart.h"
#include "osc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NTP_SYNC	2
#define SET_ALARM 	4

#ifndef NULL
#define NULL ((void *) 0)
#endif

int putchar(int c);

// UART variables
int recv0_char = 0x00;
unsigned char recv0_flag = 0x00;
int recv1_char = 0x00;
unsigned char recv1_flag = 0x00;

// Comm
typedef struct minipkt {
	int size; // bytes of parameters
	char param[99]; // array of parameters
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
void main(void) {
	osc_init_xt2();
	WDTCTL = WDTPW + WDTHOLD;
	uart1_init_p1();
	uart0_init_p1();
	_EINT();
	_BIS_SR(GIE); 
	flush_rxpkt();
}

// UART ----------------------------------

// eliminate when i2c and sd are implemented
int putchar(int c) {
	uart0_send(c);
	return c;
}

void options() {
	if (buffpkt.ready != 1) return;
	switch (buffpkt.intr) {
	case NTP_SYNC: 
		// fill with setup rtc method
		printf("hour: %d \t minute: \t %d second: %d\n", (int)buffpkt.param[0], (int)buffpkt.param[1], (int)buffpkt.param[2]); // eliminate when i2c and sd are implemented
		break;
	case SET_ALARM:
		// fill with alarm variables and method
		printf("hour: %d \t minute: %d\n", (int)buffpkt.param[0], (int)buffpkt.param[1]); // eliminate when i2c and sd are implemented
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
		buffpkt.param[buffpkt.size++] = input;
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