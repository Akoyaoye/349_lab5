/**
 * @file 
 *
 * @brief      
 *
 * @date       
 *
 * @author     
 */

#include <unistd.h>
#include <rcc.h>
#include <uart.h>
#include <uart_polling.h>
#include <nvic.h>
#include <stdbool.h>
#include <gpio.h>

#define UNUSED __attribute__((unused))

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

typedef struct {
  char data[UART_BUFFER_SIZE];
  volatile uint32_t put_ptr;
  volatile uint32_t get_ptr;
  volatile uint32_t count;
} buffer_struct;

typedef buffer_struct *buffer_t;

/** @brief Initializes buffers */
static buffer_struct tx_buffer;
static buffer_struct rx_buffer;


bool buf_empty(buffer_t buf){
  return (buf->count == 0);
}

bool buf_full(buffer_t buf){
  return (buf->count == UART_BUFFER_SIZE);
}

/** @brief Inserts byte into buffer; Returns 0 on success, -1 on failure */
int buf_put(buffer_t buf, char c){

  if (!buf_full(buf)){
    buf->data[buf->put_ptr] = c;

    //Ensure wraparound; handout say no use mod
    if (buf->put_ptr + 1 == UART_BUFFER_SIZE){
      buf->put_ptr = 0;
    } else {
      buf->put_ptr = buf->put_ptr + 1;
    }
    
    buf->count += 1; 
    return 0;
  } 

  return -1;
}

/** @brief Gets byte from buffer; Returns 0 on success, -1 on failure */
int buf_get(buffer_t buf, char* c){
  if (c == NULL) return -1;

  if (!buf_empty(buf)){
    *c = buf->data[buf->get_ptr]; 
    
    //Ensure wraparound; handout say no use mod
    if (buf->get_ptr + 1 == UART_BUFFER_SIZE){
      buf->get_ptr = 0;
    } else {
      buf->get_ptr = buf->get_ptr + 1;
    }

    buf->count -= 1; 
    return 0;
  }

  return -1;
}


void uart_init(int baud){
  struct uart_reg_map *uart = UART2_BASE;

  //Enable UART Internal Clock 
  struct rcc_reg_map *rcc = RCC_BASE;  
  rcc->apb1_enr |= RCC_UART2_CLOCK_EN;

  //Set BRR to baud rate
  uart->BRR = baud;

  //PA_2, UART TX
  gpio_init(GPIO_A, GPIO_PIN_2, MODE_ALT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, 
            PUPD_NONE, ALT7);
  //PA_3, UART RX
  gpio_init(GPIO_A, GPIO_PIN_3, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, 
            PUPD_NONE, ALT7);
  
  //Enable Transmit and Receive 
  uart->CR1 |= UART_RECEIVER_ENABLE;
  uart->CR1 |= UART_TRANSMITTER_ENABLE;
  uart->CR1 |= UART_EN;  
  
  //Enable RXNE interrupt (this is fine since rx_buf is empty)
  uart->CR1 |= UART_RXNE_ENABLE;

  //Disable TXE interrupt (since tx_buf is empty intially)
  uart->CR1 &= UART_TXEIE_DISABLE;
  
  //Enable nvic
  nvic_irq(USART2_IRQ_NUM, IRQ_ENABLE);
  return;

}

//Copies character into TX buffer
int uart_put_byte(char c){
  struct uart_reg_map *uart = UART2_BASE;

  //Disable TXE interrupt (since interrupt could fire while buf_put is running)
  nvic_irq(USART2_IRQ_NUM, IRQ_DISABLE);

  //Put byte into TX buffer 
  if (!buf_full(&tx_buffer)){
    buf_put(&tx_buffer, c);
  } else {
    uart->CR1 |= UART_TXEIE_ENABLE;
    nvic_irq(USART2_IRQ_NUM, IRQ_ENABLE);
    return -1;
  }

  uart->CR1 |= UART_TXEIE_ENABLE; //Buffer not empty so we can enable TX interrupt
  nvic_irq(USART2_IRQ_NUM, IRQ_ENABLE);
  return 0;
}

//intended to be used in external files 
int rx_remove_char(){
  nvic_irq(USART2_IRQ_NUM, IRQ_DISABLE); //mutex
  if (buf_empty(&rx_buffer)){
    nvic_irq(USART2_IRQ_NUM, IRQ_ENABLE);
    return -1;
  } 

  if (rx_buffer.put_ptr == 0) {
    rx_buffer.put_ptr = (UART_BUFFER_SIZE - 1); //subtracted 1
  }
  else{
    rx_buffer.put_ptr--; //subtracted 1
  }
  rx_buffer.count--;
  nvic_irq(USART2_IRQ_NUM, IRQ_ENABLE);

  return 1;
}

//Retrieves from RX buffer 
int uart_get_byte(char *c){
  struct uart_reg_map *uart = UART2_BASE;

  //Disable RXNE interrupt (Same reasoning as put_byte)
  // nvic_irq(USART2_IRQ_NUM, IRQ_DISABLE);
  uart->CR1 &= UART_RXNE_DISABLE; 
  
  //Gets byte from RX buffer 
  if (!buf_empty(&rx_buffer)){
    buf_get(&rx_buffer, c);
  } else {
    uart->CR1 |= UART_RXNE_ENABLE;
    // nvic_irq(USART2_IRQ_NUM, IRQ_ENABLE);
    return -1;
  }
  
  uart->CR1 |= UART_RXNE_ENABLE; //Buffer not full so we can enable RX interrupt 
  // nvic_irq(USART2_IRQ_NUM, IRQ_ENABLE);
  return 0;
}

void USART2_TX_IRQHandler(){
  struct uart_reg_map *uart = UART2_BASE;
  char c; 
  
  //Gets byte from TX buffer and puts it into uart DR
  if (!buf_empty(&tx_buffer)){ 
    buf_get(&tx_buffer, &c);
    uart->DR = c;
    if (buf_empty(&tx_buffer)) {
      uart->CR1 &= UART_TXEIE_DISABLE; //Disable TXE interrupt (since tx_buf is empty)
    }
  } 
  
}

void USART2_RX_IRQHandler(){
  struct uart_reg_map *uart = UART2_BASE;
 
  //Gets byte from uart DR and puts into RX buffer 
  if (!buf_full(&rx_buffer)){
    uart->CR1 &= UART_RXNE_DISABLE; 
    char c = (char)uart->DR;
    buf_put(&rx_buffer, c);

    if (buf_full(&rx_buffer)) {
      uart->CR1 &= UART_RXNE_DISABLE; //Disable RX interrupts (since rx_buf is full)
    } else {
      uart->CR1 |= UART_RXNE_ENABLE;
    }
  }
}


void uart_irq_handler(){
  struct uart_reg_map *uart = UART2_BASE;
  
  if (((uart->SR >> TRANSMIT_DATA_BIT) & 0x1) && 
      ((uart->CR1 >> UART_TXEIE_ENABLE_BIT) & 0x1)){
    USART2_TX_IRQHandler();
  }

  if (((uart->SR >> READ_DATA_BIT) & 0x1) && 
      ((uart->CR1 >> UART_RXNE_ENABLE_BIT) & 0x1)){
    USART2_RX_IRQHandler();
  }

  //Clear interrupt pending bit
  nvic_clear_pending(USART2_IRQ_NUM); //Assuming UART2

}

void uart_flush(){
  struct uart_reg_map *uart = UART2_BASE;
  char c;
  
  //no more interrupts ?
  while(1){
    if (buf_empty(&tx_buffer)) break;

    buf_get(&tx_buffer, &c);
    while (!((uart->SR >> TRANSMIT_DATA_BIT) & 0x1)){
      //uart_polling.c reference 
    }
    uart->DR = c;
  }
}