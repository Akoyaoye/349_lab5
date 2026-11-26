/**
 * @file
 *
 * @brief
 *
 * @date
 *
 * @author
 */

#ifndef _UART_H_
#define _UART_H_

/** @brief IRQ number for USART2 */
#define USART2_IRQ_NUM (38)

/** @brief Enable mask for TXE Interrupt Enable */
#define UART_TXEIE_ENABLE (1<<7)

/** @brief Enable bit for RXNE Interrupt Enable */
#define UART_TXEIE_ENABLE_BIT (7)

/** @brief Disable mask for TXE Interrupt */
#define UART_TXEIE_DISABLE (0xFFFFFF7F)

/** @brief Enable mask for RXNE Interrupt Enable */
#define UART_RXNE_ENABLE (1<<5)

/** @brief Disable mask for RXNE Interrupt */
#define UART_RXNE_DISABLE (0xFFFFFFDF)

/** @brief Enable bit for RXNE Interrupt Enable */
#define UART_RXNE_ENABLE_BIT (5)

/** @brief Precomputed register input for 115200 baud rate */
#define BAUD_115200 0x8B // (16Mhz/(16*115200)) 8, [11/16 = 11 = B]



/** @brief Size of circular buffer */
#define UART_BUFFER_SIZE (64)

void uart_init(int baud);

int uart_put_byte(char c);

int uart_get_byte(char *c);

int buffer_init(int size);

int uart_write( int file, char *ptr, int len );

int uart_read(int file, char *ptr, int len );

void uart_flush();

int rx_remove_char();

#endif /* _UART_H_ */
