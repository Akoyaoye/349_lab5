// James Zhang
#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>

/** @brief Base address of the I2C */
#define I2C_BASE    (struct i2c_reg_map *) 0x40005400

/** @brief Enable Bit for I2C RCC Clock En*/
#define RCC_I2C_CLOCK_EN (1<<21) 

/** @brief Mask for Peripheral clock frequency*/
#define I2C_CLK_MASK (0xFFC0)

/** @brief Enable bit for 7 bit addressing*/
#define I2C_7BADDR (0x7FFF)

/** @brief Enable bit for starting I2C*/
#define I2C_START (1<<8)

/** @brief Enable bit for stopping I2C*/
#define I2C_STOP (1<<9)

/** @brief Mask for setting address*/
#define I2C_ADDR_MASK (0xFF01)

/** @brief Mask for checking ADDR bit for transmission*/
#define I2C_ADDR (1<<1)

/** @brief Mask for checking TxE bit*/
#define I2C_TxE (1<<7)

/** @brief Mask for DR*/
#define I2C_DR_MASK (0x00)

/** @brief Mask for checking BTF bit*/
#define I2C_BTF (1<<2)

#define I2C_TRISE (1<<14)

#define I2C_CR1_MASK (1<<15)

/** @brief Mask to clear 12 LSB of CCR Register*/
#define I2C_CCR_MASK (0xF000)

#define I2C_SM_MASK (0x7FFF)

#define I2C_AF (1<<10)

#define I2C_CLOCK_80 80

#define I2C_RISE_17 17

#define I2C_CLOCK_16 (uint16_t)16

#define GPIO_PIN_8 (unsigned int)8

#define GPIO_PIN_9 (unsigned int)9

void i2c_master_init(uint16_t clk);

void i2c_master_start();

void i2c_master_stop();

int i2c_master_write(uint8_t *buf, uint16_t len, uint8_t slave_addr);

int i2c_master_read(uint8_t *buf, uint16_t len, uint8_t slave_addr);

#endif /* _I2C_H_ */
