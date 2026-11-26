#ifndef _LCD_DRIVER_H_
#define _LCD_DRIVER_H_

#include <unistd.h>

#define LCD_ADDR 0x27
#define INSTRUCTION_ENABLE_ON 0b1100
#define INSTRUCTION_ENABLE_OFF 0b1000
#define DATA_ENABLE_ON 0b1101
#define DATA_ENABLE_OFF 0b1001

#define ROW_OFFSET 0x40

#define DATA_LEN 4
#define CLEAR_LEN 4
#define DDRAM_ADDR_LEN 4
#define INIT_LEN 4 //CURRENRLY UNUSED

#define KEYPAD_STAR '*'
#define KEYPAD_POUND '#'
#define NEWLINE_CHAR '\n'


void lcd_driver_init();
void lcd_print(char *input);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_clear();

#endif /* _LCD_DRIVER_H_ */
