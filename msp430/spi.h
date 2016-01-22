#ifndef _SPI_H_
#define _SPI_H_

#include <msp430x16x.h>

void spi_initialize(void);
void spi_set_divisor(unsigned int divisor);
void spi_cs_assert(void);
void spi_cs_deassert(void);

void spi_send_byte(unsigned char input);
unsigned char spi_recv_byte(void);

void spi_enable(void);
void spi_disable(void);

#endif