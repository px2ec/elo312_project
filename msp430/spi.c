#include <msp430x16x.h>
#include "spi.h"

/* Initialize and enable the SPI module */
void spi_initialize(void)
{
	/* P3 for SPI mode */
	P3SEL |= 0x0E;
	/* P3.4 as the SS signal, active low. So, initialize it high */
	P3OUT |= 0x10;
	/* P3.4 as an output */
	P3DIR |= 0x10;

	/* 8-bit, SPI, Master, Enable SW reset */
	U0CTL = SWRST;
	U0CTL |= CHAR | SYNC | MM;
	/* SMCLK, 3-wire, Normal polarity */
	U0TCTL = SSEL1 | SSEL0 | STC | CKPH;

	/*
	 * USART Baud rate control registers
	 * SPICLK = SMCLK/2 (2: minimum divisor)
	 */
	U0BR0 = 0x02;
	U0BR1 = 0x00;
	/* USART Modulation control register */
	U0MCTL = 0x00;

	/* Module enable */
	ME1 |= USPIE0;

	/* SPI enable */
	U0CTL &= ~SWRST;
}

/*
 * Set the baud rate divisor. The correct value is computed by dividing
 * the clock rate by the desired baud rate. The minimum divisor allowed is 2.
 */
void spi_set_divisor(unsigned int divisor)
{
	U0CTL |= SWRST;
	U0BR0 = divisor;
	U0BR1 = divisor >> 8;
	U0CTL &= ~SWRST;
}

/* Assert the CS signal, active low (CS = 0) */
void spi_cs_assert(void)
{
	/* Pin 3.4 */
	P3OUT &= ~0x10;
}

/* Assert the CS signal, active high (CS = 1) */
void spi_cs_deassert(void)
{
	/* Pin 3.4 */
	P3OUT |= 0x10;
}

/* Send a single byte over the SPI port */
void spi_send_byte(unsigned char input)
{
	IFG1 &= ~URXIFG0;
	/* Send the byte */
	TXBUF0 = input;
	/* Wait for the byte to be sent */
	while ((IFG1 & URXIFG0) == 0);
}

/* Receive a byte. Output should be 0xFF to receive the bytes */
unsigned char spi_recv_byte(void)
{
	unsigned char tmp;
	IFG1 &= ~URXIFG0;
	/* Send the byte */
	TXBUF0 = 0xFF;
	/* Wait for the byte to be received */
	while ((IFG1 & URXIFG0) == 0);

	tmp = U0RXBUF;
	return tmp;	
}

/*
 * Disable the SPI module. This function assumes the module had
 * already been initialized.
 */
void spi_disable(void)
{
	/* Put the SPI module in reset mode */
	U0CTL |= SWRST;
	/* Disable the USART module */
	ME1 &= ~USPIE0;
}

/* Enable the SPI module */
void spi_enable(void)
{
	/* Enable the USART module */
	ME1 |= USPIE0;
	/* Take the SPI module out of reset mode */
	U0CTL &= ~SWRST;
}