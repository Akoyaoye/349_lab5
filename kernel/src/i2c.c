#include <unistd.h>

#include <gpio.h>
#include <i2c.h>
#include <rcc.h>
#include <printk.h>


/** @brief The I2C register map. (from m4_reference_manual) */
struct i2c_reg_map {          /**< Starts at 0x40005400 */
    volatile uint32_t CR1;    /**< Control register 1 */
    volatile uint32_t CR2;    /**< Control register 2 */
    volatile uint32_t OAR1;   /**< Own address register 1*/
    volatile uint32_t OAR2;   /**< Own address register 2*/
    volatile uint32_t DR;     /**< Data register */
    volatile uint32_t SR1;    /**< Status register 1 */
    volatile uint32_t SR2;    /**< Status register 2 */
    volatile uint32_t CCR;    /**< Clock control register */
    volatile uint32_t TRISE;  /**< TRISE register */
    volatile uint32_t FLTR;   /**< FLTR register*/
};



void i2c_master_init(uint16_t clk){
    struct i2c_reg_map *i2c = I2C_BASE;
    struct rcc_reg_map *rcc = RCC_BASE;
    
    //Enable I2C internal clock
    rcc-> apb1_enr |= RCC_I2C_CLOCK_EN; 
    
    //Initialize SDA (D14)
    gpio_init(GPIO_B, GPIO_PIN_9, MODE_ALT, OUTPUT_OPEN_DRAIN, 
              OUTPUT_SPEED_LOW ,PUPD_NONE, ALT4);

    //Initialize SCL (D15)
    gpio_init(GPIO_B, GPIO_PIN_8, MODE_ALT, OUTPUT_OPEN_DRAIN, 
              OUTPUT_SPEED_LOW ,PUPD_NONE, ALT4);
    
    //Force a reset to clear any stuck BUSY flag from GPIO-transition glitches
    i2c->CR1 |= (I2C_CR1_MASK);   
    i2c->CR1 &= ~(I2C_CR1_MASK);  
    
    //Set Peripheral clock frequenty to clk; should be 16MHz
    i2c->CR2 = (i2c->CR2 & I2C_CLK_MASK)|((uint8_t)clk);

    //Choose standard mode I2C
    i2c->CCR = i2c->CCR & I2C_SM_MASK; 

    //Configure I2C clock speed to 80 
    i2c->CCR = (i2c->CCR &  I2C_CCR_MASK)|(I2C_CLOCK_80);

    //Set Maximum RISE time to 17, as last 6 bits
    i2c->TRISE = (i2c->TRISE & 0xFFC0)|(I2C_RISE_17);

    //Use 7-bit addressing  
    i2c->OAR1 &= I2C_7BADDR;
    i2c->OAR1 |= (I2C_TRISE); 
    
    //Enable Peripheral 
    i2c->CR1 |= (0x01); 

    return;
}

void i2c_master_start() {
    struct i2c_reg_map *i2c = I2C_BASE;
    
    //Set start bit
    i2c->CR1 |= I2C_START;

    //Wait for EV5
    while(!(i2c->SR1 & 0x1)){
        
    }

    //Need to read SR1 register to clear SB bit
    i2c->SR1 |= i2c->SR1;
}

void i2c_master_stop() {
    struct i2c_reg_map *i2c = I2C_BASE;
    
    //Set stop bit
    i2c->CR1 |= I2C_STOP;

    //Wait for Hardware to clear STOP bit
    while (i2c->CR1 & I2C_STOP) {

    } 
}

int i2c_master_write(uint8_t *buf, uint16_t len, uint8_t slave_addr){
    struct i2c_reg_map *i2c = I2C_BASE;
    
    //Start I2C transmission
    i2c_master_start();

    //Put slave_addr in DR
    i2c->DR = (i2c->DR & I2C_DR_MASK)|((slave_addr << 1));
    
    //Wait for EV6 bit 
    while (!(i2c->SR1 & I2C_ADDR)){
        //Stop if address doesnt get acknowledged
        if (i2c->SR1 & I2C_AF) {
            i2c->SR1 &= ~I2C_AF;
            i2c_master_stop();
            return -1;
        }
    }

    //Need to clear ADDR by reading SR1 then SR2
    i2c->SR1 |= i2c->SR1;
    i2c->SR2 |= i2c->SR2;

    //Wait for EV8_1
    while (!(i2c->SR1 & I2C_TxE)){

    }
    
    //Transmit messages byte by byte
    for (int i = 0; i < len; i++){
        //Write data_i into DR
        i2c->DR = (i2c->DR & I2C_DR_MASK)|(buf[i]);

        //DR gets emptied immediately, so poll for EV8 
        while (!(i2c->SR1 & I2C_TxE)){

        }
    }

    //Wait for EV8_2
    while((!(i2c->SR1 & I2C_TxE))||(!(i2c->SR1 & I2C_BTF))){

    }
    
    //Stop I2C transmission
    i2c_master_stop();

    return 0;
}

int i2c_master_read(uint8_t *buf, uint16_t len, uint8_t slave_addr){
    (void) buf;
    (void) len;
    (void) slave_addr;

    return 0;
}
