#include <unistd.h>

#include <gpio.h>
#include <keypad_driver.h>
#include <systick.h>


void keypad_init() {
    //cols (to be pulled down)
    gpio_init(KEYPAD_COL1_PORT, KEYPAD_COL1_PIN, MODE_GP_OUTPUT, 
              OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
    gpio_init(KEYPAD_COL2_PORT, KEYPAD_COL2_PIN, MODE_GP_OUTPUT, 
              OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
    gpio_init(KEYPAD_COL3_PORT, KEYPAD_COL3_PIN, MODE_GP_OUTPUT, 
              OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);

    //rows (to be read, either connected to ground or pulled high)
    gpio_init(KEYPAD_ROW1_PORT, KEYPAD_ROW1_PIN, MODE_INPUT, OUTPUT_PUSH_PULL, 
              OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);
    gpio_init(KEYPAD_ROW2_PORT, KEYPAD_ROW2_PIN, MODE_INPUT, OUTPUT_PUSH_PULL, 
              OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);
    gpio_init(KEYPAD_ROW3_PORT, KEYPAD_ROW3_PIN, MODE_INPUT, OUTPUT_PUSH_PULL, 
              OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);
    gpio_init(KEYPAD_ROW4_PORT, KEYPAD_ROW4_PIN, MODE_INPUT, OUTPUT_PUSH_PULL, 
              OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);
}

// Just using gpio_read, but easier via case statement
int read_row(int row){
    switch (row){
        case ROW_1:
            return gpio_read(KEYPAD_ROW1_PORT, KEYPAD_ROW1_PIN);
        case ROW_2:
            return gpio_read(KEYPAD_ROW2_PORT, KEYPAD_ROW2_PIN);
        case ROW_3:
            return gpio_read(KEYPAD_ROW3_PORT, KEYPAD_ROW3_PIN);
        case ROW_4:
            return gpio_read(KEYPAD_ROW4_PORT, KEYPAD_ROW4_PIN);
        default: 
            return -1;
    }
}

void set_col(int col){
    //We want to drive one low and set others to high
    //lowk had a case statement earlier but realized ts was stupid

    if (col == COL_1){
        gpio_clr(KEYPAD_COL1_PORT, KEYPAD_COL1_PIN);
    } 
    else{
        gpio_set(KEYPAD_COL1_PORT, KEYPAD_COL1_PIN);
    }

    if (col == COL_2){
        gpio_clr(KEYPAD_COL2_PORT, KEYPAD_COL2_PIN);
    }
    else{
        gpio_set(KEYPAD_COL2_PORT, KEYPAD_COL2_PIN);
    }

    if (col == COL_3){
        gpio_clr(KEYPAD_COL3_PORT, KEYPAD_COL3_PIN);
    }
    else{
        gpio_set(KEYPAD_COL3_PORT, KEYPAD_COL3_PIN);
    }

    return;
}

//just busy waits, cant rely on unix sleep apparently 
void wasteful_sleep(int time) {
    for (int i = 0; i < time * 8000; i++) {

    }
}

//Set a col, the search each row to see if low (default high), if the row is high, 
//Check which col is activated, and return the intersection
char keypad_read() {
    int col;
    int row;
    for (col = 1; col <= NUM_COLS; col++){
        set_col(col);
        for (row = 1; row <= NUM_ROWS; row++){
            if (read_row(row) == 0){ //This row is low, one of buttons in it is pressed
                systick_delay(100); //delay for some time to allow debouncing
                switch (row){
                    case ROW_1:
                        if (col == COL_1) return COL1_ROW1;
                        if (col == COL_2) return COL2_ROW1;
                        if (col == COL_3) return COL3_ROW1;
                        break;
                    case ROW_2:
                        if (col == COL_1) return COL1_ROW2;
                        if (col == COL_2) return COL2_ROW2;
                        if (col == COL_3) return COL3_ROW2;
                        break;
                    case ROW_3:
                        if (col == COL_1) return COL1_ROW3;
                        if (col == COL_2) return COL2_ROW3;
                        if (col == COL_3) return COL3_ROW3;
                        break;
                    case ROW_4:
                        if (col == COL_1) return COL1_ROW4;
                        if (col == COL_2) return COL2_ROW4;
                        if (col == COL_3) return COL3_ROW4;
                        break;  
                }
            }
        }
    }
    return '\0';
}
