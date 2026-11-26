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
#include <timer.h>
#include <rcc.h>
#include <nvic.h>
#include <gpio.h> 
#include <servok.h> 
#include <systick.h>
#include <printk.h>
#include <stdio.h>

#define UNUSED __attribute__((unused))

/** @brief tim2_5 */
struct tim2_5 {
  volatile uint32_t cr1; /**< 00 Control Register 1 */
  volatile uint32_t cr2; /**< 04 Control Register 2 */
  volatile uint32_t smcr; /**< 08 Slave Mode Control */
  volatile uint32_t dier; /**< 0C DMA/Interrupt Enable */
  volatile uint32_t sr; /**< 10 Status Register */
  volatile uint32_t egr; /**< 14 Event Generation */
  volatile uint32_t ccmr[2]; /**< 18-1C Capture/Compare Mode */
  volatile uint32_t ccer; /**< 20 Capture/Compare Enable */
  volatile uint32_t cnt; /**< 24 Counter Register */
  volatile uint32_t psc; /**< 28 Prescaler Register */
  volatile uint32_t arr; /**< 2C Auto-Reload Register */
  volatile uint32_t reserved_1; /**< 30 */
  volatile uint32_t ccr[4]; /**< 34-40 Capture/Compare */
  volatile uint32_t reserved_2; /**< 44 */
  volatile uint32_t dcr; /**< 48 DMA Control Register */
  volatile uint32_t dmar; /**< 4C DMA address for full transfer Register */
  volatile uint32_t or; /**< 50 Option Register */
};

struct tim2_5* const timer_base[] = {(void *)0x0,   // N/A - Don't fill out
                                     (void *)0x0,   // N/A - Don't fill out
                                     (void *)0x40000000,  //m4_ref pg 39
                                     (void *)0x40000400,  //m4_ref pg 39
                                     (void *)0x40000800,  //m4_ref pg 39
                                     (void *)0x40000C00}; //m4_ref pg 39


void timer_init(int timer, uint32_t prescalar, uint32_t period) {
  struct rcc_reg_map *rcc = RCC_BASE;

  switch (timer){
    case TIM2_INT_NUM:
      rcc->apb1_enr |= TIM2_EN;

      struct tim2_5 *tim_2 = timer_base[2];
      tim_2->psc = prescalar - 1; 
      tim_2->arr = period - 1; //read systick.c
      
      tim_2->cr1 |= 1;

      tim_2->egr  |= 1; //Update generation bit
      tim_2->sr    = 0; //clear the UIF that UG just set
      tim_2->dier |= 1; //enable interrupt bit 

      nvic_irq(TIM2_IRQ, IRQ_ENABLE);
      //might need to gen reg and clear the interrupt bit it fires
       // start count
      return;
    
    case TIM3_INT_NUM:
      rcc->apb1_enr |= TIM3_EN;

      struct tim2_5 *tim_3 = timer_base[3];
      tim_3->psc = prescalar - 1; 
      tim_3->arr = period - 1; //read systick.c

      tim_3->cr1 |= 1; 

      tim_3->egr  |= 1; //Update generation bit

      
      tim_3->sr    = 0; //clear the UIF that UG just set

      nvic_clear_pending(TIM3_IRQ);

      tim_3->dier |= 1; //enable interrupt bit 

      nvic_irq(TIM3_IRQ, IRQ_ENABLE);
      //might need to gen reg and clear the interrupt bit it fires
      // start count
      break;

    case TIM4_INT_NUM:
      rcc->apb1_enr |= TIM4_EN;

      struct tim2_5 *tim_4 = timer_base[4];
      tim_4->psc = prescalar - 1; 
      tim_4->arr = period - 1; //read systick.c

      tim_4->cr1 |= 1;

      tim_4->egr  |= 1; //Update generation bit
      tim_4->sr    = 0; //clear the UIF that UG just set

      nvic_clear_pending(TIM4_IRQ);

      tim_4->dier |= 1; //enable interrupt bit 
  
      nvic_irq(TIM4_IRQ, IRQ_ENABLE);
      //might need to gen reg and clear the interrupt bit it fires
       // start count
      break;

    case TIM5_INT_NUM:
      rcc->apb1_enr |= TIM5_EN;

      struct tim2_5 *tim_5 = timer_base[5];
      tim_5->psc = prescalar - 1; 
      tim_5->arr = period - 1; //read systick.c

      tim_5->cr1 |= 1;

      tim_5->egr  |= 1; //Update generation bit
      tim_5->sr    = 0; //clear the UIF that UG just set
      tim_5->dier |= 1; //enable interrupt bit 

      nvic_irq(TIM5_IRQ, IRQ_ENABLE);
      //might need to gen reg and clear the interrupt bit it fires
      // start count
      break;
  }
}

void timer_disable(int timer) {
  switch (timer){
    case TIM2_INT_NUM:
      struct tim2_5 *tim_2 = timer_base[2];
      tim_2->cr1 &= ~1; //disable counter
      return; 

    case TIM3_INT_NUM:
      struct tim2_5 *tim_3 = timer_base[3];
      tim_3->cr1 &= ~1; //disable counter
      return; 

    case TIM4_INT_NUM:
      struct tim2_5 *tim_4 = timer_base[4];
      tim_4->cr1 &= ~1; //disable counter
      return; 

    case TIM5_INT_NUM:
      struct tim2_5 *tim_5 = timer_base[5];
      tim_5->cr1 &= ~1; //disable counter
      return;
  }
}

void timer_clear_interrupt_bit(UNUSED int timer) {
  switch (timer){
    case TIM2_INT_NUM:
      struct tim2_5 *tim_2 = timer_base[2];
      tim_2->sr &= ~1; //set update inter flag back to 0
      return; 

    case TIM3_INT_NUM:
      struct tim2_5 *tim_3 = timer_base[3];
      tim_3->sr &= ~1; //set update inter flag back to 0
      return; 

    case TIM4_INT_NUM:
      struct tim2_5 *tim_4 = timer_base[4];
      tim_4->sr &= ~1; //set update inter flag back to 0
      return; 

    case TIM5_INT_NUM:
      struct tim2_5 *tim_5 = timer_base[5];
      tim_5->sr &= ~1; //set update inter flag back to 0
      return;
  }
} 

void tim2_c_handler(void){
  timer_clear_interrupt_bit(TIM2_INT_NUM);

  //Initialize another timer so gpio can be set low after (x)ms & set GPIO pin high
  if (ch0_en){
    timer_init(TIM3_INT_NUM, 160, (pwm_channel0 + 60));
    gpio_set(GPIO_A, 0);
  } 

  if (ch1_en){
    timer_init(TIM4_INT_NUM, 160, (pwm_channel1 + 60));
    gpio_set(GPIO_A, 1);
  }
  
}

void tim3_c_handler(void){
  //Clear gpio pins to 0 and disable timer
  timer_clear_interrupt_bit(TIM3_INT_NUM);

  gpio_clr(GPIO_A, 0);
  
  timer_disable(TIM3_INT_NUM);
  
}

void tim4_c_handler(void){
  timer_clear_interrupt_bit(TIM4_INT_NUM);

  gpio_clr(GPIO_A, 1);
  
  timer_disable(TIM4_INT_NUM);
  
}


void tim5_c_handler(void){
  timer_clear_interrupt_bit(TIM5_INT_NUM);  

  static int on = 0;
  on = !on;
  if (on) gpio_set(GPIO_A, 0);
  else    gpio_clr(GPIO_A, 0);
}