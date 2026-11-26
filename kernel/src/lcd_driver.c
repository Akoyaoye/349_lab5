#include <unistd.h>

#include <i2c.h>
#include <lcd_driver.h>
#include <keypad_driver.h>
#include <printk.h>
#include <systick.h>

//static Variables:
static uint8_t LCD_CURSOR_ROW = 0;
static uint8_t LCD_CURSOR_COL = 0;


void lcd_driver_init() {
    //refer to page 46 of HD44780 for full control flow
    uint8_t init_array[INIT_LEN];

    // instruction is 0011 and set the conditions accordingly: RS=0 (instruction), etc
    systick_delay(16); //delay 16 ms
    init_array[0] = (0b0011 << 4) | (INSTRUCTION_ENABLE_ON);
    init_array[1] = (0b0011 << 4) | (INSTRUCTION_ENABLE_OFF); //have to do again but turn off enable
    i2c_master_write(init_array, INIT_LEN, LCD_ADDR);

    //send next instruction (same)
    systick_delay(5); // delay > 4.1 ms
    init_array[0] = (0b0011 << 4) | (INSTRUCTION_ENABLE_ON);
    init_array[1] = (0b0011 << 4) | (INSTRUCTION_ENABLE_OFF); //have to do again but turn off enable
    i2c_master_write(init_array, INIT_LEN, LCD_ADDR);

    //send next instruction (same again)
    systick_delay(101); //delay > 100 ms
    init_array[0] = (0b0011 << 4) | (INSTRUCTION_ENABLE_ON);
    init_array[1] = (0b0011 << 4) | (INSTRUCTION_ENABLE_OFF); //have to do again but turn off enable
    i2c_master_write(init_array, INIT_LEN, LCD_ADDR);

    //send last (not same, top bit i(s 0010)
    init_array[0] = (0b0010 << 4) | (INSTRUCTION_ENABLE_ON);
    init_array[1] = (0b0010 << 4) | (INSTRUCTION_ENABLE_OFF); //have to do again but turn off enable
    i2c_master_write(init_array, INIT_LEN, LCD_ADDR);


    //set display control things, from page 24 of HD44780
    systick_delay(2000); //delay 2s for clear 
    init_array[0] = (0b0000 << 4) | (INSTRUCTION_ENABLE_ON);
    init_array[1] = (0b0000 << 4) | (INSTRUCTION_ENABLE_OFF);
    i2c_master_write(init_array, INIT_LEN, LCD_ADDR);

    init_array[0] = (0b1110 << 4) | (INSTRUCTION_ENABLE_ON);
    init_array[1] = (0b1110 << 4) | (INSTRUCTION_ENABLE_OFF);
    i2c_master_write(init_array, INIT_LEN, LCD_ADDR);

	return;
}


//@requires input[\length(input) - 1] == '\0';
void lcd_print(char *input){
    int i = 0; 
    uint8_t use;
    uint8_t upper;
    uint8_t lower;
    uint8_t data[DATA_LEN];

    while((use = input[i]) != '\0'){
        i++;
        
        switch (use){
            case KEYPAD_STAR:
                LCD_CURSOR_ROW = (LCD_CURSOR_ROW == 0) ? 1 : 0; //set row to 1 if currently 0, else set to 1
                break;
            case KEYPAD_POUND:
                lcd_clear();
                break;
            default:
                upper = ((use >> 4) & 0xF);
                lower = (use & 0xF);

                data[0] = (upper << 4) | (DATA_ENABLE_ON);  
                data[1] = (upper << 4) | (DATA_ENABLE_OFF); 
                data[2] = (lower << 4) | (DATA_ENABLE_ON);  
                data[3] = (lower << 4) | (DATA_ENABLE_OFF);

                i2c_master_write(data, DATA_LEN, LCD_ADDR);

                LCD_CURSOR_COL++;
                if (LCD_CURSOR_COL >= 16){ //Overflow
                    LCD_CURSOR_COL = 0;
                    LCD_CURSOR_ROW++; //go to next row
                }
        } 
        lcd_set_cursor(LCD_CURSOR_ROW, LCD_CURSOR_COL);
    }
}

void lcd_set_cursor(uint8_t row, uint8_t col){
    //first, send the DDRAM address that we are printing to: 16char, 2 line
    //base row = 0x0, base col = 0x40
    uint8_t addr = (row * ROW_OFFSET) + col; //row 1, col 0 = 0x40, (rowx, col) offset by 0x40
    uint8_t ddram_addr[DDRAM_ADDR_LEN]; //base address found in HD44780 page 12 == 0x0
    
    uint8_t full = (0b1 << 7) | (addr & 0b01111111); //top 1 bits hardset
    uint8_t top_bits = ((full >> 4) & 0xF);
    uint8_t bot_bits = (full & 0xF);

    ddram_addr[0] = (top_bits << 4) | (INSTRUCTION_ENABLE_ON); //D7 are hardcoded (not addr fields)
    ddram_addr[1] = (top_bits << 4) | (INSTRUCTION_ENABLE_OFF); //have to do again but turn off enable

    //send lower 4 bits (D3-D0)
    ddram_addr[2] = ((bot_bits) << 4) | (INSTRUCTION_ENABLE_ON);
    ddram_addr[3] = ((bot_bits) << 4) | (INSTRUCTION_ENABLE_OFF); //have to do again but turn off enable

    //send
    i2c_master_write(ddram_addr, DDRAM_ADDR_LEN, LCD_ADDR);
}

void lcd_clear() {
    uint8_t clear_display[CLEAR_LEN];
    clear_display[0] = (0b0000 << 4) | (INSTRUCTION_ENABLE_ON);
    clear_display[1] = (0b0000 << 4) | (INSTRUCTION_ENABLE_OFF); 
    clear_display[2] = (0b0001 << 4) | (INSTRUCTION_ENABLE_ON);
    clear_display[3] = (0b0001 << 4) | (INSTRUCTION_ENABLE_OFF);
    i2c_master_write(clear_display, CLEAR_LEN, LCD_ADDR);

    //wait for it to actually clear
    systick_delay(2000); // 2 seconds

    LCD_CURSOR_ROW = 0;
    LCD_CURSOR_COL = 0;
    lcd_set_cursor(LCD_CURSOR_ROW, LCD_CURSOR_COL);
}
