/**
 * @file     kernel.c
 *
 * @brief    Kernel entry point
 *
 */

#include "kernel.h"
#include "arm.h"
#include "motor_driver.h"

int kernel_main(void) {
    init_349(); // DO NOT REMOVE THIS LINE

    enter_user_mode();
    while (1) {
    }
    return 0;
}
