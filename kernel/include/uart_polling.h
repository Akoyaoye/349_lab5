#ifndef _UART_POLLING_H_
#define _UART_POLLING_H_


#define TRANSMIT_DATA_BIT 7
#define READ_DATA_BIT 5

/** @brief Base address for UART2 */
#define UART2_BASE  (struct uart_reg_map *) 0x40004400

/** @brief Enable  Bit for UART Config register */
#define UART_EN (1 << 13)

/** @brief Enable  Bit for UART RCC Clock En */
#define RCC_UART2_CLOCK_EN (1<<17)

/** @brief Precomputed register input for 115200 baud rate */
#define BAUD_115200 0x8B // (16Mhz/(16*115200)) 8, [11/16 = 11 = B]

/** @brief Enable  Bit for UART CR1 register reciver enable */
#define UART_RECEIVER_ENABLE (1<<2) 

/** @brief Enable  Bit for UART CR1 register transmitter enable */
#define UART_TRANSMITTER_ENABLE (1<<3) 

#define GPIO_PIN_2 2
#define GPIO_PIN_3 3

void uart_polling_init (int baud);
void uart_polling_put_byte (char c);
char uart_polling_get_byte ();

#endif /* _UART_POLLING_H_ */
