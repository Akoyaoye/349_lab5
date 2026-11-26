/** @file   syscall_thread.c
 *
 *  @brief  
 *
 *  @date   
 *
 *  @author 
 */

#include <stdint.h>
#include "syscall_thread.h"
#include "syscall_mutex.h"
#include <stdlib.h>
#include <mpu.h>
#include <systick.h>
#include <arm.h>
#include <string.h>
#include <stdio.h>
#include <syscall.h>
#include <stdbool.h>



#define MUTEX_MAX_NUM 32
#define TCB_MAX_NUM 33    //calculated but adjustable
#define BLOCKED_NONE 42
#define LOCKED_NONE 42

/** @brief      Initial XPSR value, all 0s except thumb bit. */
#define XPSR_INIT 0x1000000

/** @brief Interrupt return code to user mode using PSP.*/
#define LR_RETURN_TO_USER_PSP 0xFFFFFFFD
/** @brief Interrupt return code to kernel mode using MSP.*/
#define LR_RETURN_TO_KERNEL_MSP 0xFFFFFFF1

#define THREAD_DEFAULT 0
#define THREAD_IDLE 1
#define THREAD_FIRST_USER 2

extern void thread_kill(void);
extern void idle_default(void);

/**
 * @brief      Heap high and low pointers.
 */
//@{
extern char
  __thread_u_stacks_low,
  __thread_u_stacks_top,
  __thread_k_stacks_low,
  __thread_k_stacks_top;
//@}

/**
 * @brief      Precalculated values for UB test.
 */
float ub_table[] = {
  0.000, 1.000, .8284, .7798, .7568,
  .7435, .7348, .7286, .7241, .7205,
  .7177, .7155, .7136, .7119, .7106,
  .7094, .7083, .7075, .7066, .7059,
  .7052, .7047, .7042, .7037, .7033,
  .7028, .7025, .7021, .7018, .7015,
  .7012, .7009
};

/**
 * @struct user_stack_frame
 *
 * @brief  Stack frame upon exception.
 */
typedef struct {
  uint32_t r0;   /** @brief Register value for r0 */
  uint32_t r1;   /** @brief Register value for r1 */
  uint32_t r2;   /** @brief Register value for r2 */
  uint32_t r3;   /** @brief Register value for r3 */
  uint32_t r12;  /** @brief Register value for r12 */
  uint32_t lr;   /** @brief Register value for lr*/
  uint32_t pc;   /** @brief Register value for pc */
  uint32_t xPSR; /** @brief Register value for xPSR */
} interrupt_stack_frame;


kernel_info kernel;
TCB tcb_storage[TCB_MAX_NUM]; //static alloc
kmutex_t mutex_array[MUTEX_MAX_NUM];



TCB *find_next_thread(void){
  for (uint32_t i = THREAD_FIRST_USER; i < kernel.max_threads + 2; i++){
    if (kernel.thread_array[i].thread_status == INIT ||
        kernel.thread_array[i].thread_status == TERMINATED)
      return &kernel.thread_array[i];
  }
  return NULL; //its over
} 

kmutex_t *find_next_mutex(void){
  for (uint32_t i = 0; i < MUTEX_MAX_NUM; i++){
    if (mutex_array[i].mutex_status == EMPTY){
      mutex_array[i].index = i; //we dont keep i 
      return &mutex_array[i];
    }
  }
  return NULL; //its over
} 


void *pendsv_c_handler(void *context_ptr){
  TCB *curr_thread = &kernel.thread_array[kernel.current];
  TCB *t;
  uint32_t least;
  uint32_t least_index;
  bool elevated;

  least = 50000; //Idc, this priority doesnt exist (shuld prolly make it macro or smth)
  curr_thread->msp = (uint32_t*)context_ptr; //update msp in TCB
  curr_thread->svc_status = get_svc_status();

  
  if (curr_thread->thread_status == RUNNING){
    curr_thread->thread_status = RUNNABLE; 
  }

  //Find least priority in RUNNABLE pool
  for (uint32_t i = 2; i < kernel.max_threads + 2; i++){
    t = &(kernel.thread_array)[i];
    elevated = (t->base_priority != t->priority);
    if (((t->priority < least) && (t->thread_status == RUNNABLE))||
        (elevated && (t->priority == least))){
      least = t->priority;
      least_index = i;
    }
  }
   
  //Update if one was found
  if (least != 50000){
    t = &((kernel.thread_array)[least_index]);
    t->thread_status = RUNNING; 
    kernel.current = least_index;

    set_svc_status(t->svc_status); 

    return (void*)t->msp; 
  }

   
  
  if (kernel.num_threads == 0) { //Run default
    kernel.current = 0;
    (&(kernel.thread_array[0]))->thread_status = RUNNING; 

    set_svc_status(kernel.thread_array[0].svc_status); 

    return (void*)((&kernel.thread_array[0])->msp);

  } else { //Run idle
    kernel.current = 1;
    (&(kernel.thread_array[1]))->thread_status = RUNNING; 

    set_svc_status(kernel.thread_array[1].svc_status);

    return (void*)((&kernel.thread_array[1])->msp);
  }
  
}


void init_default_thread(void) {
  TCB *t = &kernel.thread_array[THREAD_DEFAULT];
  t->thread_status = RUNNING;
  t->exc_return = LR_RETURN_TO_USER_PSP;
  t->T = UINT32_MAX;
}


void init_idle_thread(void *idle_fn){ 
  TCB *idle = &kernel.thread_array[THREAD_IDLE]; //idle thread hardcoded to 1

  interrupt_stack_frame *f = (interrupt_stack_frame *)(idle->thread_u_stack_begin - 8);  

  if (idle_fn == NULL) idle_fn = idle_default;
  f->pc = (uint32_t)idle_fn;  
  f->lr = (uint32_t)thread_kill;                
  f->xPSR = XPSR_INIT;      
                        
  //stack da pendsv
  idle->msp = idle->thread_k_stack_begin - 10;
  *(idle->msp) = (uint32_t)f;                    //r0, context_ptr            
  *(idle->msp + 9) = LR_RETURN_TO_USER_PSP;      //lr              

  idle->priority = kernel.max_threads;
  idle->T = UINT32_MAX;
  idle->psp = (uint32_t *)f;      
  idle->thread_status = RUNNABLE; 
}


int sys_thread_init(
  uint32_t max_threads,
  uint32_t stack_size,
  void *idle_fn,
  uint32_t max_mutexes
){
  if (max_threads == 0) return -1;
  
  if (max_threads + 2 > TCB_MAX_NUM) return -1;
  memset(tcb_storage, 0, sizeof(tcb_storage));    //zero the struct array
  TCB *thread_array_global = tcb_storage;         //devolution of array to ptr

  //need to round up user stack space:
  if (stack_size < 256) stack_size = 256;
  if (stack_size > 8192) return -1; //not enough space
  stack_size = 1 << mm_log2ceil_size(stack_size);

  //check the threads have enough space t
  uint32_t slots = max_threads + 1; //adding idle thread
  uint32_t u_words = (uint32_t *)&__thread_u_stacks_top - (uint32_t *)&__thread_u_stacks_low; //distance between the ptrs in words
  uint32_t k_words = (uint32_t *)&__thread_k_stacks_top - (uint32_t *)&__thread_k_stacks_low;
  if ((slots * stack_size > u_words) || (slots * stack_size > k_words)) return -1;


  //do the relevant things within the array
  for (uint32_t i = THREAD_IDLE; i < max_threads + 2; i++){
    (thread_array_global[i]).thread_status = INIT;

    //get addr of the top of the stack for this given thread, stack_size given is 4 bytes, cast uint32_t (!!)
    uint32_t *thread_stack_space = ((uint32_t *)&__thread_u_stacks_top - ((i-1) * stack_size)); //i - 1 as thread 0 dosent get its (own) stack space
    (thread_array_global[i]).psp = thread_stack_space;
    (thread_array_global[i]).thread_u_stack_begin = thread_stack_space;

    //same for msp:
    uint32_t *thread_kernel_space = ((uint32_t *)&__thread_k_stacks_top - ((i-1) * stack_size));
    (thread_array_global[i]).msp = thread_kernel_space;
    (thread_array_global[i]).thread_k_stack_begin = thread_kernel_space;

    
    (thread_array_global[i]).exc_return = LR_RETURN_TO_USER_PSP;
  }

  //kernel state things (global)
  kernel.max_threads = max_threads;
  kernel.stack_size = stack_size;
  kernel.num_threads = 0;
  kernel.tick_count = 0;
  kernel.current = THREAD_DEFAULT; 
  kernel.thread_array = thread_array_global;

  init_default_thread();
  init_idle_thread(idle_fn);
  
  (void) max_mutexes; //extra shit
  return 0;
}

int sys_thread_create(
  void *fn,
  uint32_t prio,
  uint32_t C,
  uint32_t T,
  void *vargp
){
  TCB *thread = find_next_thread();
  if (thread == NULL) return -1; //there are too many threads gng 

  interrupt_stack_frame *regs = (interrupt_stack_frame *)(thread->thread_u_stack_begin - 8); 
  regs->r0 = (uint32_t)vargp;
  regs->pc = (uint32_t)fn;
  regs->xPSR = (uint32_t)XPSR_INIT;
  regs->lr = (uint32_t)thread_kill; //page 11 of writeup 

  thread->msp = thread->thread_k_stack_begin - 10;      //grow msp for shit
  *(thread->msp) = (uint32_t)regs;                      // r0 -> becomes PSP
  *(thread->msp + 9) = LR_RETURN_TO_USER_PSP;           // lr
  thread->svc_status = 0; 

  //may have to zero regs in psp and msp

  //check priorities dont overlap
  for (uint32_t i = THREAD_FIRST_USER; i < kernel.max_threads + 2; i++) {
    TCB *t = &kernel.thread_array[i];
    if ((t->thread_status != INIT) && (t->thread_status != TERMINATED) && (t->priority == prio))
      return -1;
  }

  thread->priority = prio;
  thread->base_priority = prio; //added MUTEX
  thread->C = C;
  thread->T = T;
  thread->psp = (uint32_t *)regs; 
  thread->exc_return = LR_RETURN_TO_USER_PSP; 
  thread->blocking_mutex = BLOCKED_NONE;    //added for MUTEX

  thread->ticks_this_period = 0;
  thread->total_ticks = 0;
  thread->next_wake = kernel.tick_count + T; //wake up after Ts after now 
  thread->thread_status = RUNNABLE;

  //Check schedulability through RMS 
  uint32_t n = 0; 
  float sum = 0;
  TCB *t; 

  for (uint32_t i = THREAD_FIRST_USER; i < kernel.max_threads + 2; i++){
    t = &(kernel.thread_array)[i];
    if (t->thread_status == RUNNABLE || 
        t->thread_status == WAITING || 
        t->thread_status == RUNNING){
          sum += (float)(t->C)/(t->T);
          n += 1;
    }
  }
  
  //If not schedulable, revert thread status and return -1
  if (sum > ub_table[n]){
    thread->thread_status = INIT; 
    return -1; 
  }
  
  kernel.num_threads++; 
  return 0;
}

int sys_scheduler_start(uint32_t frequency){
  if (frequency == 0) return -1;
  if (kernel.num_threads == 0) return 0; 


  kernel.tick_count = 0;
  for (uint32_t i = THREAD_FIRST_USER; i <= kernel.max_threads + 1; i++){
    TCB *t = &kernel.thread_array[i];
    
    if (t->thread_status == RUNNABLE){
      t->next_wake = t->T;  
      t->ticks_this_period = 0;
    }
  }

  kernel.thread_array[THREAD_DEFAULT].thread_status = WAITING;
  systick_init(frequency);

  pend_pendsv();

  return 0; //need to worry about what happens here
}

uint32_t sys_get_priority(){

  TCB *curr_thread = &kernel.thread_array[kernel.current];
  return (uint32_t)(curr_thread->priority);

}

uint32_t sys_get_time(){
  return (uint32_t)(kernel.tick_count);
}

uint32_t sys_thread_time(){
  return (uint32_t)((&kernel.thread_array[kernel.current])->total_ticks);
}


void sys_thread_kill(uint32_t *psp){
  
	//If default thread called kill -> exit 
  if (kernel.current == THREAD_DEFAULT) {
    sys_exit(-1);    
    return;      
  }

  if (kernel.current == THREAD_IDLE){
    psp[6] = (uint32_t)&idle_default; //pc
    return;
  }

  TCB *curr_thread = &kernel.thread_array[kernel.current];
  curr_thread->thread_status = TERMINATED;
  kernel.num_threads--;
  
  //Stop systick if no runnable threads available
  if (kernel.num_threads == 0){
    systick_stop();
  }

  pend_pendsv(); 
}

void sys_wait_until_next_period(){
  if (kernel.current == THREAD_IDLE) return;

  TCB *t = &kernel.thread_array[kernel.current];
  t->thread_status = WAITING;

  pend_pendsv();
}
 
kmutex_t *sys_mutex_init( uint32_t max_prio ) {
  kmutex_t *mutex;
  if ((mutex = find_next_mutex()) == NULL) return NULL;

  mutex->prio_ceil = max_prio;
  mutex->mutex_status = UNLOCKED;
  mutex->locked_by = LOCKED_NONE;

  return mutex; 
}

/**
*@brief This function is what is called when a thread attempts to lock a mutex.
*       the function checks the priority invaraint, and allows for the thread to be
*       blocked or to claim the mutex, depending on if the mutex has already been claimed
*/
void sys_mutex_lock( kmutex_t *mutex ) {
  TCB *current = &kernel.thread_array[kernel.current];

  //check if the prio ceiling was set correctly
  if (current->base_priority < mutex->prio_ceil) { //TODO make this encapsulated 
    TCB *curr_thread = &kernel.thread_array[kernel.current];
    curr_thread->thread_status = TERMINATED;
    kernel.num_threads--;
    pend_pendsv();
    return;   
  } 

  //if locked by current thread, wtf r we doing
  if (mutex->locked_by == kernel.current) return;
  
  //claim mutex (?)
  if (mutex->mutex_status == UNLOCKED) {
    mutex->mutex_status = LOCKED;
    mutex->locked_by = kernel.current;

    if (current->priority > mutex->prio_ceil) current->priority = mutex->prio_ceil;  //added MUTEX

    return;
  }

  if (mutex->mutex_status == LOCKED){
    current->blocking_mutex = mutex->index;
    current->thread_status = BLOCKED;
    pend_pendsv(); //This thread is blocked, find one that isnt (such as the thread that has this mutex, maybe? fn)
    return;
  }
}

/**
*  @brief the mutex that is held by the thread that called this is unlocked,
*         and crucially, only the next highest prio thread is removed from the blocked list,
*         not every thread that is blocked by the mutex (they are still blocked)
*/
void sys_mutex_unlock(kmutex_t *mutex) {
  if (mutex->locked_by != kernel.current) return; //saftey (test)

  uint32_t least_prio = (kernel.thread_array[kernel.current]).base_priority;

  //find the next highest mutex for hlp to demote the thread to that prio (base if none found, O(N)) <- TODO
  for (uint32_t i = 0; i < MUTEX_MAX_NUM; i++){
    if ((mutex_array[i].locked_by == kernel.current) &&
        (&mutex_array[i] != mutex) && (mutex_array[i].prio_ceil < least_prio)){
      least_prio = mutex_array[i].prio_ceil;
    }
  }

  (kernel.thread_array[kernel.current]).priority = least_prio;


  // find the highest prio thread blocked (that isnt this one)
  int32_t best_index = -1; 
  uint32_t best_prio = UINT32_MAX;        

  
  for (long unsigned int i = THREAD_FIRST_USER; i < kernel.max_threads + 2; i++) {
    TCB *thread = &kernel.thread_array[i];

    if ((thread->thread_status == BLOCKED) && (thread->blocking_mutex == mutex->index)) {
      if (thread->priority < best_prio) { 
        best_prio = thread->priority;
        best_index  = i;
      }
    }
  }

  if (best_index == -1) { //No other threads were blocked by this mutex
    mutex->mutex_status = UNLOCKED;
    mutex->locked_by = LOCKED_NONE;
  } 
   else { //thread found, release it!
    TCB *best = &kernel.thread_array[best_index];
    best->blocking_mutex = BLOCKED_NONE;
    best->thread_status = RUNNABLE;
    mutex->locked_by = best_index;
    if (best->priority > mutex->prio_ceil) best->priority = mutex->prio_ceil;
  }
  pend_pendsv();
}