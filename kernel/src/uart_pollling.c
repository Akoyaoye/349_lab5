#include <unistd.h>

#include <gpio.h>
#include <rcc.h>
#include <uart_polling.h>

/** @brief The UART register map. */
struct uart_reg_map {
    volatile uint32_t SR;   /**< Status Register */
    volatile uint32_t DR;   /**<  Data Register */
    volatile uint32_t BRR;  /**<  Baud Rate Register */
    volatile uint32_t CR1;  /**<  Control Register 1 */
    volatile uint32_t CR2;  /**<  Control Register 2 */
    volatile uint32_t CR3;  /**<  Control Register 3 */
    volatile uint32_t GTPR; /**<  Guard Time and Prescaler Register */
};



/**
 * @brief initializes UART to given baud rate with 8-bit word length, 1 stop bit, 0 parity bits
 *
 * @param baud Baud rate
 */
void uart_polling_init (int baud){
    struct uart_reg_map *uart = UART2_BASE;

    //rcc base address 
    struct rcc_reg_map *rcc = RCC_BASE;  //use struct to acess area of memory
    rcc->apb1_enr |= RCC_UART2_CLOCK_EN; //enable uart2 clock (bit 17)

    //Simply set BRR to baud rate, chill
    uart->BRR = baud;

    //PA_2, UART TX , TODO: FIX MAGIC NUM (!)
    gpio_init(GPIO_A, GPIO_PIN_2, MODE_ALT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, 
              PUPD_NONE, ALT7);
    //PA_3, UART RX
    gpio_init(GPIO_A, GPIO_PIN_3, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, 
              PUPD_NONE, ALT7);

    //Enable transmit and Recieve 
    uart->CR1 |= UART_RECEIVER_ENABLE;
    uart->CR1 |= UART_TRANSMITTER_ENABLE;
    uart->CR1 |= UART_EN;    

    return;
}

/**
 * @brief transmits a byte over UART
 *
 * @param c character to be sent
 */
void uart_polling_put_byte (char c){
    struct uart_reg_map *uart = UART2_BASE;

    while (!((uart->SR >> TRANSMIT_DATA_BIT) & 0x1)){

    }

    uart->DR = c;
    return;
}

/**
 * @brief receives a byte over UART
 */
char uart_polling_get_byte () {
    struct uart_reg_map *uart = UART2_BASE;

    while (!((uart->SR >> READ_DATA_BIT) & 0x1)){
        
    }

    return (char)uart->DR;
}
