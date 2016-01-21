#ifndef __UART_H__
#define __UART_H__

#include <msp430.h>

extern /* volatile */ int recv0_char;
extern /* volatile */ unsigned char recv0_flag;

extern /* volatile */ int recv1_char;
extern /* volatile */ unsigned char recv1_flag;

void uart0_init_p1(void);
void uart0_init_p2(void);
int  uart0_send(int c);
int  uart0_recv(void);
void uart0_sendstr(char*);
void uart0_sendarr(char*, int);

void uart1_init_p1(void);
//void uart1_init_p2(void);
int  uart1_send(int c);
int  uart1_recv(void);
void uart1_sendstr(char*);
void uart1_sendarr(char*, int);


extern void *(*uart0_int_recv_callback)(void *);
extern void *(*uart1_int_recv_callback)(void *);

#endif