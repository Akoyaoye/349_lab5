#ifndef _KEYPAD_DRIVER_H_
#define _KEYPAD_DRIVER_H_

#include <unistd.h>
#include <gpio.h>

// Macros for Keypad GPIO pins 
#define KEYPAD_COL1_PORT GPIO_A
#define KEYPAD_COL1_PIN 7

#define KEYPAD_COL2_PORT GPIO_A
#define KEYPAD_COL2_PIN 5

#define KEYPAD_COL3_PORT GPIO_B
#define KEYPAD_COL3_PIN 6

#define KEYPAD_ROW1_PORT GPIO_C
#define KEYPAD_ROW1_PIN 7

#define KEYPAD_ROW2_PORT GPIO_C
#define KEYPAD_ROW2_PIN 10

#define KEYPAD_ROW3_PORT GPIO_C
#define KEYPAD_ROW3_PIN 12

#define KEYPAD_ROW4_PORT GPIO_C
#define KEYPAD_ROW4_PIN 11

#define ROW_1 (int)1
#define ROW_2 (int)2
#define ROW_3 (int)3
#define ROW_4 (int)4
#define NUM_ROWS (int)4

#define COL_1 (int)1
#define COL_2 (int)2
#define COL_3 (int)3
#define NUM_COLS (int)3

#define COL1_ROW1 (char)'1'
#define COL2_ROW1 (char)'2'
#define COL3_ROW1 (char)'3'

#define COL1_ROW2 (char)'4'
#define COL2_ROW2 (char)'5'
#define COL3_ROW2 (char)'6'

#define COL1_ROW3 (char)'7'
#define COL2_ROW3 (char)'8'
#define COL3_ROW3 (char)'9'

#define COL1_ROW4 (char)'*'
#define COL2_ROW4 (char)'0'
#define COL3_ROW4 (char)'#'
/*
 * keypad_init: Initialize the keypad.
 */
void keypad_init();

/*
 * read_row: Read the value set by a certain row
 */
int read_row(int row);

/*
 * set_col: set a column to high
 */
void set_col(int col);

/*
 * sleep for a set ammount of time (no units attached)
 */
void wasteful_sleep(int time);

/*
 * keypad_read: Return the current character that is being pressed. If
 * no character is pressed, return '\0'
 *
 * For example, if you press 1 and call this function, it should return '1'!
 * Don't worry about handing two or more buttons pressed at once.
 */
char keypad_read();

#endif /* _KEYPAD_DRIVER_H_ */
