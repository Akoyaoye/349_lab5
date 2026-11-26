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
#include <syscall.h>
#include <uart.h>
#include <lcd_driver.h>
#include <arm.h>
#include <printk.h>
// #include <int_to_string.h>

#define UNUSED __attribute__((unused))

extern char __heap_low; //Linker script
extern char __heap_top; //Linker script

void *sys_sbrk(int incr){
  static char *program_break = &__heap_low;
  void *former_break = (void *)program_break;

  if (incr < 0) return (void *)-1;

  size_t space_remaining = (size_t)(&__heap_top - program_break); //both heap top and program break are 1 byte
  if ((size_t)space_remaining < (size_t)incr) return (void *)-1;

  program_break += (size_t)incr;
  
  return former_break;
}

int sys_write(int file, char *ptr, int len){
  if (file != STDOUT_FILENO) return -1; 
  if (len < 0) return -1;

  for (int i = 0; i < len; i++) {
    while (uart_put_byte(ptr[i]) == -1) { //buffer currently full
      // wait_for_interrupt();  //lowwk a little racey, will get to it if cause problems 
    }
  }

  return len;
}

int sys_read(int file, char *ptr, int len){
  //we only have one "file"
  if (file != STDIN_FILENO) return -1;
  if (ptr == NULL) return -1;

  char c;
  int i = 0;
  while (i < len){
    

    while (uart_get_byte(&c) == -1){
      // wait_for_interrupt();
    }

    //  __asm("bkpt");

    
    switch (c){
      case 4: //eof character
        return i;
        break; //not needed but sure
      
      case '\b':
        if (i > 0) i--; //the index of the thing b4 will be overwritten
        while(uart_put_byte('\b') != 0);
        while(uart_put_byte(' ') != 0);
        while(uart_put_byte('\b') != 0);
        break;
      
      case '\r':
      case '\n':
        ptr[i] = '\n';
        while(uart_put_byte('\n') != 0);
        i++;
        return i;
        break; //once again, not needed but whatever 
      
      default:
        ptr[i] = c;
        i++;
        while(uart_put_byte(c) != 0);
        break;
    }
  }
 

  return i;
}

void sys_exit(int status){
  //print status code
  printk("exit code: %d", status);

  //flush buffer 
  disable_interrupts();
  uart_flush(); 


  //print code to led, assuming led has already been initialized 
  // char buf[NUM_BITS_INTkr         ];
  // char *input = int_to_str(status, buf);
  // lcd_print(input);
  while(1){
     wait_for_interrupt();
  }
 
}
