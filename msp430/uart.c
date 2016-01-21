/*
 * Nombre del modulo: uart.c
 *
 * Modulo creado para la asignatura ELO-312
 * Laboratorio de Estructura de Computadores
 *
 * Departamento de Electrónica
 * Universidad Técnica Federico Santa María
 *
 * El uso o copia está permitido y se agracede mantener
 * el nombre de los creadores.
 *
 * 25/11/2013 - Mauricio Solís
 *
 * Descripción del módulo:
 * Modulo driver UART
 * Contiene las funciones que permiten manejar el
 * modulo UART.
 */

#include "uart.h"
/* Puerto 1 UART/ USART

/*
 * Inicializa el modulo UART a una velocidad de app 9600 baudios
 * con iterrupción de recepción
 */
void uart0_init_p1(void) {
	P3SEL |= 0x30;
	ME1 |= UTXE0 + URXE0;
	UCTL0 |= CHAR;
	UTCTL0 |= 0x10; /* SMCLK */
	UBR00 = 0x03;
	UBR10 = 0x00;
	UMCTL0 = 0x4A;
	UCTL0 &= ~SWRST;
	IE1 |= URXIE0;
}

int uart0_send(int c) {
	while (!(IFG1 & UTXIFG0));
		TXBUF0 = (unsigned char)c;
	return c;
}

int uart0_recv(void) {
	int retval = 0;
	while (1) {
		if (U0RCTL & RXERR) {
			U0RCTL &= ~(FE+PE+OE+BRK+RXERR);
		} else {
			if (IFG1 & URXIFG0) {
				retval = (int)U0RXBUF;
				break;
			}
		}
	}
	return retval;
}

void uart0_sendstr(char *buf) {
	int i = 0;
	for (i = 0; buf[i] != '\0'; i++)
		uart0_send(buf[i]);
}

void uart0_sendarr(char *buf, int size) {
	int i = 0;
	for (i = 0; i < size; i++)
		uart0_send(buf[i]);
}

#pragma vector = UART0RX_VECTOR
__interrupt void uart0_int_recv(void) {
	recv0_char = (int)U0RXBUF;
	recv0_flag = 0;
	uart0_int_recv_callback(&recv0_char);
}
/* ---------------------------------------------------------------------------------------------------*/

/* Puerto 2 UART/USART

/*
 * Inicializa el modulo UART a una velocidad de app 9600 baudios
 * con iterrupción de recepción
 */
void uart1_init_p1(void) {
	P3SEL |= 0xC0;
	ME2 |= UTXE1 + URXE1;
	UCTL1 |= CHAR;
	UTCTL1 |= 0x10; /* SMCLK */
	UBR01 = 0x03;
	UBR11 = 0x00;
	UMCTL1 = 0x4A;
	UCTL1 &= ~SWRST;
	IE2 |= URXIE1;
}

/*
 * Inicializa el modulo UART a una velocidad de 2400 baudios
 * con iterrupción de recepción
 */
 /*
void uart1_init_p2(void) {
	P3SEL |= 0xC0;
	ME2 |= UTXE1 + URXE1;
	UCTL1 |= CHAR;
	UTCTL1 |= SSEL0;
	UBR01 = 0x0D;
	UBR11 = 0x00;
	UMCTL1 = 0x6B;
	UCTL1 &= ~SWRST;
	IE2 |= URXIE1;
}*/

int uart1_send(int c) {
	while (!(IFG2 & UTXIFG1));
		TXBUF1 = (unsigned char)c;
	return c;
}

int uart1_recv(void) {
	int retval = 0;
	while (1) {
		if (U1RCTL & RXERR) {
			U1RCTL &= ~(FE+PE+OE+BRK+RXERR);
		} else {
			if (IFG2 & URXIFG1) {
				retval = (int)U1RXBUF;
				break;
			}
		}
	}
	return retval;
}

void uart1_sendstr(char *buf) {
	int i = 0;
	for (i = 0; buf[i] != '\0'; i++)
		uart0_send(buf[i]);
}

void uart1_sendarr(char *buf, int size) {
	int i = 0;
	for (i = 0; i < size; i++)
		uart0_send(buf[i]);
}

#pragma vector = UART1RX_VECTOR
__interrupt void uart1_int_recv(void) {
	recv1_char = (int)U1RXBUF;
	recv1_flag = 0;
	uart1_int_recv_callback(&recv1_char);
}