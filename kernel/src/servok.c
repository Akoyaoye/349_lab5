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
#include <syscall.h>
#include <gpio.h> 
#include <timer.h>

#define UNUSED __attribute__((unused))

//default angles
volatile int pwm_channel0 = 90;   
volatile int pwm_channel1 = 90;
volatile int ch0_en = 0;
volatile int ch1_en = 0;

int sys_servo_enable(uint8_t channel, uint8_t enabled){
  //Channel must be either 0 or 1
  if (channel > 1){
      return -1;
  }

  //Initialize GPIO pin for PWM
  if (channel == 0){
    gpio_init(GPIO_A, channel, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, 
            OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
    gpio_clr(GPIO_A, channel);
  } 
  
  if (channel == 1){
    gpio_init(GPIO_A, channel, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, 
            OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
    gpio_clr(GPIO_A, channel);
  }
  
  
  
  if (enabled){
    //Update global variables
    if (channel == 0) {ch0_en = 1;} 
    if (channel == 1) {ch1_en = 1;} 

    //Initialize timer to pulse every 20ms
    timer_init(TIM2_INT_NUM, 160, 2000);
    
  } else {
    if (channel == 0) {ch0_en = 0;}
    if (channel == 1) {ch1_en = 0;}
  }

  return 0;
}

int sys_servo_set(uint8_t channel, uint8_t angle){
  if (channel > 1){
      return -1;
  }

  if (angle > 180){
    return -1;
  }

  if (channel == 0){pwm_channel0 = angle;} 
  if (channel == 1){pwm_channel1 = angle;}

  return 0; 
}