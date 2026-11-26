#ifndef _TIMER_H_
#define _TIMER_H_

/** @brief  The processor frequency (16MHz) */
#define BASE_FREQ 16000000

#define TIM2_INT_NUM    28
#define TIM3_INT_NUM    29
#define TIM4_INT_NUM    30
#define TIM5_INT_NUM    50

#define TIM2_EN (1 << 0)
#define TIM3_EN (1 << 1)
#define TIM4_EN (1 << 2)
#define TIM5_EN (1 << 3)

#define TIM2_IRQ 28
#define TIM3_IRQ 29
#define TIM4_IRQ 30
#define TIM5_IRQ 50

#include <unistd.h>

/*
 * Starts the timer
 *
 * @param timer      - The timer
 * @param prescaler  - Prescalar for clock
 * @param Period     - Period of the timer interrupt
 */
void timer_init(int timer, uint32_t prescalar, uint32_t period);

/*
 * Stops the timer
 *
 * @param timer      - The timer
 */
void timer_disable(int timer);

/*
 * Clears the timer interrupt bit
 *
 * @param timer      - The timer
 */
void timer_clear_interrupt_bit(int timer);

#endif /* _TIMER_H_ */
