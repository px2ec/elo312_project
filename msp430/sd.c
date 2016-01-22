#include <msp430x16x.h>
#include "types.h"
#include "spi.h"
#include "sd.h"
#include "sdcomm_spi.h"

static unsigned char response[5];
static unsigned char argument[4];
static u8 blank = 0xFF;

int sd_card_present(void)
{
	return !(P3IN & 0x01);
}

/*
 * This function initializes the SD card. It returns 1 if
 * initialization was successful, 0 otherwise.
 *
 * sd_context_t *sdc -- Pointer to a data structure containing
 *                      information about the card. For now, the
 *                      timeouts must be specified in advance. It
 *                      is not yet calculated from the card data.
 */
int sd_initialize(sd_context_t *sdc)
{
	u16 i;

	/*
	 * SPI SDv2 initialization sequence:
	 *   CMD0
	 *   CMD8
	 *   CMD55+ACMD41
	 *   CMD58
	 *     (There is no CMD2 or CMD3 in SPI mode. These
	 *      instructions are devoted to addressing on the SD bus)
	 */

	sdc->busyflag = 0;

	/*
	 * Delay for at least 74 clock cycles. This means to actually
	 * *clock* out at least 74 clock cycles with no data present on
	 * the clock. In SPI mode, send at least 10 idle bytes (0xFF)
	 */
	spi_cs_deassert();
	sd_delay(100);
	spi_cs_assert();
	sd_delay(2);

	/* Put the card in thee idle state */
	if (sd_send_command(sdc, CMD0, CMD0_R, response, NULL) == 0)
		return 0;

	sd_packarg(argument, 0x000001AA);
	if (sd_send_command(sdc, CMD8, CMD8_R, response, argument) == 0) {
		return 0;
	}

	if (response[0] & MSK_ILL_CMD) {
		/* Illegal command, try SDv1 init */
		sd_packarg(argument, 0x00000000);
	} else {
		/* Check if the voltage range is appropiate */
		if (response[3] == 0x01 && response[4] == 0xAA) {
			sd_packarg(argument, 0x40000000);
		} else {
			return 0;
		}
	}

	/* Now wait until the card goes idle. Retry at most SD_IDLE_WAIT_MAX */
	i = 0;
	do {
		i++;
		/* Flag the next command as an application-specific command */
		if (sd_send_command(sdc, CMD55, CMD55_R, response, NULL) == 1) {
			/* Tell the card to send its OCR */
			sd_send_command(sdc, ACMD41, ACMD41_R, response, argument);
		} else {
			/* No response, bail early */
			i = SD_IDLE_WAIT_MAX;
		}
	} while ((response[0] & MSK_IDLE) == MSK_IDLE && i < SD_IDLE_WAIT_MAX);

	/* As long as we didn't hit the timeout, assume we're OK */
	if (i >= SD_IDLE_WAIT_MAX)
		return 0;

	if (sd_send_command(sdc, CMD58, CMD58_R, response, NULL) == 0)
		return 0;

	/* At the very minimum, we must allow 3.3V */
	if ((response[2] & MSK_OCR_33) != MSK_OCR_33)
		return 0;

	/* Set the block length */
	if (sd_set_blocklen(sdc, SD_BLOCKSIZE) != 1)
		return 0;

	/* If we got this far, initialization was OK */
	return 1;
}

/*
 * This function reads a single block from the SD card at block
 * blockaddr. The buffer must be preallocated. Returns 1 if the
 * command was successful, zero otherwise.
 *
 * This is an ASYNCHRONOUS call. The transfer will not be complete
 * when the function returns. If you want to explicity wait until
 * any pending transfer are finished, use the function
 * sd_wait_notbusy(sdc)
 *
 * sd_context_t *sdc   -- A pointer to an sd device context structure,
 *                        populated by the function sd_initialize()
 * u32 blockaddr       -- The block address to read from the card.
 *                        This is a block address, not a linear address.
 * unsigned char *data -- The data, a pointer to an array of unsigned
 *                        chars.
 */
int sd_read_block(sd_context_t *sdc, u32 blockaddr,
	unsigned char *data)
{
	u32 i = 0;
	unsigned char tmp;

	/* Adjust the block address to a linear address */
	/* blockaddr <<= SD_BLOCKSIZE_NBITS; */

	/* Wait until any old transfers are finished */
	sd_wait_notbusy(sdc);

	/* Pack the address */
	sd_packarg(argument, blockaddr);

	/* Need to add size checking */
	if (sd_send_command(sdc, CMD17, CMD17_R, response, argument) == 0)
		return 0;

	/* Check for an error, like a misaligned read */
	if (response[0] != 0)
		return 0;

	/* Re-assert CS to continue the transfer */
	spi_cs_assert();

	/* Wait for the token */
	i = 0;
	do {
		tmp = spi_recv_byte();
		i++;
	} while ((tmp == 0xFF) && i < sdc->timeout_read);

	if ((tmp & MSK_TOK_DATAERROR) == 0) {
		/* Clock out a byte before returning */
		spi_send_byte(0xFF);
		/* The card returned an error response. Ball and return 0 */
		return 0;
	}

	IFG1 &= ~URXIFG0;
	IFG1 &= ~UTXIFG0;

	/* Get the block */
	/* Source DMA address: receive register */
	DMA0SA = U0RXBUF_;
	/* Destination DMA address: the user data buffer */
	DMA0DA = (unsigned int)data;
	/* The size of the block to be transfered */
	DMA0SZ = SD_BLOCKSIZE;
	/* Configure the DMA transfer */
	DMA0CTL =
		DMADT_0 |                  /* Single transfer mode */
		DMASBDB |                  /* Byte mode */
		DMAEN |                    /* Enable DMA */
		DMADSTINCR1 | DMADSTINCR0; /* Increment the destination address */

	/*
	 * We depend on the DMA priorities here. Both triggers occur at
	 * the same time, sine the source is identical. DMA0 is handled
	 * first, and retrieves the byte. DMA1 is triggered next, and
	 * sends the next byte.
	 */
	/* Source DMA address: constant 0xFF (don't increment) */
	DMA1SA = (unsigned int)&blank;
	/* Destination DMA address: the transmit buffer */
	DMA1DA = U0TXBUF_;
	/* Increment the destination address */
	/* The size of the block to be transfered */
	DMA0SZ = SD_BLOCKSIZE - 1;
	/* Configure the DMA transfer */
	DMA1CTL =
		DMADT_0 |                  /* Single transfer mode */
		DMASBDB |                  /* Byte mode */
		DMAEN;                     /* Enable DMA */

	/* DMA trigger UART receive for both DMA0 and DMA1 */
	DMACTL0 = DMA0TSEL_3 | DMA1TSEL_3;

	/* Kick off transfer by sending the first byte */
	U0TXBUF = 0xFF;

	return 1;
}

/*
 * This function writes a single block to the SD card at block
 * blockaddr. Returns 1 if the command was successful, zero
 * otherwise.
 *
 * This is an ASYNCHRONOUS call. The transfer will not be complete
 * when the function returns. If you want to explicity wait until
 * any pending transfer are finished, use the function
 * sd_wait_notbusy(sdc)
 *
 * sd_context_t *sdc   -- A pointer to an sd device context structure,
 *                        populated by the function sd_initialize()
 * u32 blockaddr       -- The block address to write to the card.
 *                        This is a block address, not a linear address.
 * unsigned char *data -- The data, a pointer to an array of unsigned
 *                        chars.
 */
int sd_write_block(sd_context_t *sdc, u32 blockaddr,
	unsigned char *data)
{

	/* Adjust the block address to a linear address */
	blockaddr <<= SD_BLOCKSIZE_NBITS;

	/* Wait until any old transfers are finished */
	sd_wait_notbusy(sdc);

	/* Pack the address */
	sd_packarg(argument, blockaddr);

	if (sd_send_command(sdc, CMD24, CMD24_R, response, argument) == 0)
		return 0;

	/* Check for an error, like a misaligned write */
	if (response[0] != 0)
		return 0;

	/* Re-assert CS to continue transfer */
	spi_cs_assert();

	/*
	 * The write command need an addition 8 lock cycles before the
	 * block write is started
	 */
	spi_recv_byte();

	/* Clear any pending flags */
	IFG1 &= ~(URXIFG0 | UTXIFG0);

	/* Get the block */
	/* Source DMA address: receive register */
	DMA0SA = (unsigned int)data;
	/* Destination DMA address: the user data buffer */
	DMA0DA = U0TXBUF_;
	/* The size of the block to be transfered */
	DMA0SZ = SD_BLOCKSIZE;
	/* Configure the DMA transfer */
	DMA0CTL =
		DMADT_0 |                  /* Single transfer mode */
		DMASBDB |                  /* Byte mode */
		DMAEN |                    /* Enable DMA */
		DMADSTINCR1 | DMADSTINCR0; /* Increment the destination address */

	/* DMA trigger UART send */
	DMACTL0 = DMA0TSEL_3;

	/* Kick off transfer by sending the first byte */
	U0TXBUF = SD_TOK_WRITE_STARTBLOCK;

	/*
	 * Signal that the card may be busy, so any subsequent commands
	 * should wait for the busy signalling to end (indicated by a
	 * nonzero response)
	 */
	sdc->busyflag = 1;

	return 1;
}

/*
 * This function synchronously waits any pending block transfers
 * are finished. If your program needs to ensure a block has finished
 * transferring, call this function.
 *
 * Note that sd_read_block() and sd_write_block() already call this
 * function internally before attempting a new transfer, so there are
 * only two times when a user would need to use this function.
 *
 * 1) When the processor will be shutting down. All pending
 *    writes should be finished first.
 * 2) When the user needs the result of an sd_read_block() call
 *    right away.
 */
void sd_wait_notbusy(sd_context_t *sdc)
{
	/* Just twiddle our thumbs until the transfer's done */
	while ((DMA0CTL & DMAEN) != 0);

	/* Reset the DMA controller */
	DMACTL0 = 0;

	/* Ignore the checksum */
	sd_delay(4);

	/* Check for the busy flag (set on write block) */
	if (sdc->busyflag == 1) {
		while (spi_recv_byte() != 0xFF);
		sdc->busyflag = 0;
	}

	/* Deassert CS */
	spi_cs_deassert();

	/*
	 * Send some extra clocks so the card can resynchronize
	 * on next transfer
	 */
	sd_delay(2);
}

void sd_packarg(unsigned char *argument, u32 value)
{
	argument[3] = (unsigned char)(value >> 24);
	argument[2] = (unsigned char)(value >> 16);
	argument[1] = (unsigned char)(value >> 8);
	argument[0] = (unsigned char)(value);
}

int sd_send_command(sd_context_t *sdc,
	unsigned char cmd, unsigned char response_type,
	unsigned char *response, unsigned char *argument)
{
	int i;
	u8 response_length;
	unsigned char tmp;

	spi_cs_assert();
	/* All data is sent MSB first and MSb first */

	/* Send the header/command */
	/*
	 * Format:
	 *   cmd[7:6] : 01
	 *   cmd[5:0] : command
	 */
	spi_send_byte((cmd & 0x3F) | 0x40);

	for (i = 3; i >= 0; i--)
		if (argument != NULL)
			spi_send_byte(argument[i]);
		else
			spi_send_byte(0x00);

	/*
	 * This is the CRC. It only matters for CMD0 and CMD8.
	 * Otherwise, the CRC is ignored for SPI mode unless we
	 * enable CRC checking.
	 */
	if (cmd == CMD0)
		spi_send_byte(0x95);
	else if (cmd == CMD8)
		spi_send_byte(0x87);
	else
		spi_send_byte(0x8E);

	response_length = 0;
	switch (response_type) {
	case R1:
	case R1B:
		response_length = 1;
		break;
	case R2:
		response_length = 2;
		break;
	case R3:
	case R7:
		response_length = 5;
		break;
	default:
		break;
	}

	/*
	 * Wait for a response. A response can be recognized by the
	 * start bit (a zero)
	 */
	i = 0;
	do {
		tmp = spi_recv_byte();
		i++;
	} while (((tmp & 0x80) != 0) && i < SD_CMD_TIMEOUT);

	/* Just bail if we never got a response */
	if (i >= SD_CMD_TIMEOUT) {
		spi_cs_deassert();
		return 0;
	}

	for (i = 0; i < response_length; i++) {
		response[i] = tmp;
		/* This handles the trailing byte requrement */
		tmp = spi_recv_byte();
	}

	/*
	 * If the response is a "busy" type (R1B), then there's some
	 * special handling that needs to be done. This card will
	 * output a continuous stream of zeros, so the end of the BUSY
	 * state is signaled by any nonzero response. The bus idles high.
	 */
	i = 0;
	if (response_type == R1B) {
		do {
			i++;
			tmp = spi_recv_byte();
			/*
			 * This should never time out, unless SDI is grounded.
			 * Don't bother forcing a timeout condition here.
			 */
		} while (tmp != 0xFF);

		spi_send_byte(0xFF);
	}

	spi_cs_deassert();
	return 1;
}

void sd_delay(u8 number)
{
	u8 i;

	for (i = 0; i < number; i++) {
		/* Clock out an idle byte (0xFF) */
		spi_send_byte(0xFF);
	}
}

int sd_set_blocklen(sd_context_t *sdc, u32 length)
{
	/* Pack the block length */
	sd_packarg(argument, length);

	return sd_send_command(sdc, CMD16, CMD16_R, response, argument);
}