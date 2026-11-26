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
#include <systick.h>
#include <arm.h>
#include <syscall_thread.h>

#define UNUSED __attribute__((unused))

#define STK_CTRL (volatile uint32_t*)0xE000E010
#define STK_LOAD (volatile uint32_t*)0xE000E014

#define KHZ 1000

volatile uint32_t global_ticks = 0;

extern kernel_info kernel;

void systick_init(uint32_t frequency) {
    *STK_LOAD &= ~(0x00FFFFFF);
    uint32_t reload_val = (BASE_FREQ / frequency) - 1;
    *STK_LOAD |= (reload_val & 0x00FFFFFF); 

    *STK_CTRL &= ~(0x7); //clear lowest 3
    *STK_CTRL |= (0x7); //enable counter, count down to zero asserts interrpy, processor clock 
}

void systick_delay(uint32_t ticks_ms) {
    uint32_t start = systick_get_ticks();

    while(global_ticks < start + ticks_ms){ //ticks no yet equal to #ticks wanted     
    
    }
}

uint32_t systick_get_ticks() {
    return global_ticks; //interrupt driven from handler
}

void systick_stop(void) {
  *STK_CTRL &= ~(0x7);
}

void systick_c_handler() {
    global_ticks++; //the pricessor counted 16k cycles = 1ms
    kernel.tick_count++;

    //Incr curr threads ticks 
    TCB *curr_thread = &kernel.thread_array[kernel.current];
    if (curr_thread != NULL) curr_thread->total_ticks += 1; 
    
    //If running thread reaches allocated time
    if (kernel.current >= 2 && curr_thread->thread_status == RUNNING){
        curr_thread->ticks_this_period += 1;
        if (curr_thread->ticks_this_period >= curr_thread->C){
            curr_thread->thread_status = WAITING;
            curr_thread->ticks_this_period = 0;
        }
    }
    
    
    //Check threads thats are in WAITING state to potentially update state to RUNNABLE
    for (uint32_t i = 2; i < kernel.max_threads + 2; i++) {
        TCB *t = &kernel.thread_array[i];

        if (t->thread_status == WAITING &&
            t->next_wake <= kernel.tick_count) {

            t->thread_status = RUNNABLE;
            t->ticks_this_period = 0;
            t->next_wake += t->T;
        }
    }
    
    pend_pendsv();
}
