/**
 * @file 
 *
 * @brief      
 *
 * @date       
 *
 * @author     
 */

#include <stdint.h>
#include <debug.h>
#include <syscall.h>
#include <svc_num.h>
#include <syscall_thread.h>
#include <servok.h>
#include <syscall_mutex.h>

#define UNUSED __attribute__((unused))

void svc_c_handler(uint32_t* psp) {
  // according to convention, pc is 6th thing pushed onto the stack 
  uint32_t *pc = (uint32_t *)(psp[6]); //get thing at the addr!

  //svc instruction is 2 bytes: (opcode, Immediate), little endian gives (im,op)
  uint8_t svc_number = *(((uint8_t *)pc) - 2); //had to rmv 2 cause of note in 
  switch (svc_number) {
    case SVC_EXIT:
      //psp points to saved r0
      int exit_status = (int)(*psp);
      sys_exit(exit_status);
      break;

    case SVC_READ:
      int read_file = (int)(*psp); //r0
      char *read_ptr = (char *)(*(psp + 1)); //r1
      int read_len = (int)(*(psp + 2)); //r2

      *psp = sys_read(read_file, read_ptr, read_len);
      break;

    case SVC_WRITE:
      int write_file = (int)(*psp); //r0
      char *write_ptr = (char *)(*(psp + 1)); //r1
      int write_len = (int)(*(psp + 2)); //r2

      *psp = sys_write(write_file, write_ptr, write_len);
      break;

    case SVC_SBRK:
      int sbrk_incr = (int)(*psp); //r0
      *psp = (uint32_t)sys_sbrk(sbrk_incr);
      break;
    
    //make the brkpt happy, idk bro
    case SVC_FSTAT: 
      break; 

    case SVC_ISATTY: 
      break;

    case SVC_SERVO_ENABLE: 
      uint8_t channel_en = (uint8_t)(*psp); //r0
      uint8_t enabled_en = (uint8_t)(*(psp + 1)); //r1
      *psp = sys_servo_enable(channel_en, enabled_en); 
      break;
    
    case SVC_SERVO_SET:
      uint8_t channel_set = (uint8_t)(*psp); //r0
      uint8_t angle_set = (uint8_t)(*(psp + 1)); //r1
      *psp = sys_servo_set(channel_set, angle_set);
      break;

    case SVC_THR_INIT:
      uint32_t max_threads = (uint32_t)(*psp); //r0
      uint32_t stack_size = (uint32_t)(*(psp + 1)); //r1
      void *idle_fn = (void*)(*(psp + 2)); //r2
      uint32_t max_mutexes = (uint32_t)(*(psp + 3)); //r3
      *psp = sys_thread_init(max_threads, stack_size, idle_fn, max_mutexes);
      break;
    
    case SVC_THR_CREATE: 
      void *fn = (void*)(*psp); //r0
      uint32_t prio = (uint32_t)(*(psp + 1)); //r1
      uint32_t C = (uint32_t)(*(psp + 2)); //r2
      uint32_t T = (uint32_t)(*(psp + 3)); //r3
      void *vargp = (void*)(*(psp + 8)); //get value from stack

      *psp = sys_thread_create(fn, prio, C, T, vargp);
      break;
    
    case SVC_THR_KILL:
      sys_thread_kill(psp);
      break; 

    case SVC_SCHD_START:
      uint32_t frequency = (uint32_t)(*psp); //r0
      *psp = sys_scheduler_start(frequency);
      break;

    case SVC_MUT_INIT:
      uint32_t max_prio = (uint32_t)(*psp); //r0
      *psp = (uint32_t)sys_mutex_init(max_prio);
      break;

    case SVC_MUT_LOK:
      kmutex_t *mutex = (kmutex_t *)(*psp); //r0
      sys_mutex_lock(mutex);
      break;

    case SVC_MUT_ULK:
      kmutex_t *mutex_ULK = (kmutex_t *)(*psp); //r0
      sys_mutex_unlock(mutex_ULK);
      break;

    case SVC_WAIT:
      sys_wait_until_next_period();
      break;

    case SVC_TIME:
      *psp = sys_get_time();
      break;

    case SVC_PRIORITY:
      *psp = sys_get_priority();
      break;
    
    case SVC_THR_TIME:
      *psp = sys_thread_time();
      break;


  default:
    DEBUG_PRINT( "Not implemented, svc num %d\n", svc_number );
    ASSERT( 0 );
  }
}