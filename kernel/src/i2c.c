#include <stdint.h>
#include <gpio.h>
#include <i2c.h>
#include <rcc.h>

/** @brief The I2C register map. */
struct i2c_reg_map {
    volatile uint32_t CR1;   //Control register 1
    volatile uint32_t CR2;   //Control register 2
    volatile uint32_t OAR1;  //Own address register 1
    volatile uint32_t OAR2;  //Own address register 2
    volatile uint32_t DR;  //Data register
    volatile uint32_t SR1;   //Status reg 1
    volatile uint32_t SR2;  //status reg 2
    volatile uint32_t CCR;  //clock control register
    volatile uint32_t TRISE;  //TRISE register
    volatile uint32_t FLTR;
};
#define I2C1_BASE (struct i2c_reg_map *) 0x40005400

#define I2C1_RCC_EN (1<<21)

#define I2C1_CR2_CLK_MASK 0x3f
#define CLK_FREQ_PERIPH 16 //in MHz

#define CCR_MASK 0xfff

//max_rise_time(ns)/clock_period(ns) + 1
//1000/62.5 + 1 = 17
#define TRISE_VAL 17
#define TRISE_MASK 0x1F

#define OAR1_B14 (1<<14)

//i2c pheripheral enable
#define CR1_PE (1<<0)

#define CR1_START (1<<8)
#define CR1_STOP (1<<9)
#define SR1_SB (1<<0)
#define SR1_ADDR (1<<1)
#define SR1_AF (1<<10)
#define SR1_TxE (1<<7)
#define SR1_BTF (1<<2)

static struct i2c_reg_map *i2c = I2C1_BASE;
/**
 * @brief initilizes I2C with SCL on D15 and SDA on D14. Sets control regs
 * and frequency
 * @param clk SCL frequency in kHz
 */
void i2c_master_init(uint16_t clk){
    //scl: d15 -> PB_8
    gpio_init(GPIO_B, 8, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, 
                PUPD_NONE, ALT4);

    //sda: d14 -> PB_9
    gpio_init(GPIO_B, 9, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, 
                PUPD_NONE, ALT4);

    struct rcc_reg_map *rcc = RCC_BASE;
    
    rcc->apb1_enr |= I2C1_RCC_EN;

    //sets pheripheral clock frequency
    i2c->CR2 |= (CLK_FREQ_PERIPH & I2C1_CR2_CLK_MASK);

    /* T_high = CCR * T_PCLK1 and T_high = T_SCL/2
     * so CCR = f_PCLK1 / (2 * f_SCL); clk is f_SCL in kHz */
    uint16_t ccr = (1000 * CLK_FREQ_PERIPH) / (2*clk);
    i2c->CCR |= (ccr & CCR_MASK); //Set scl as 100KHz

    i2c->TRISE &= ~TRISE_MASK; //clear trise
    i2c->TRISE |= (TRISE_VAL & TRISE_MASK); //set trise
    
    i2c->OAR1 |= OAR1_B14;
    
    i2c->CR1 |= CR1_PE;
}

// start sequence for I2C transmission
void i2c_master_start() {
    i2c->CR1 |= CR1_START;
    while (!(i2c->SR1 & SR1_SB));
}

// stop sequence for I2C transmission
void i2c_master_stop() {
    i2c->CR1 |= CR1_STOP;
}

/* clears AF, releases the bus, and returns the error code */
static int i2c_nack(void) {
    i2c->SR1 &= ~SR1_AF;
    i2c_master_stop();
    return -1;
}

/**
 * @brief write function for i2c
 * 
 * @param buf buffer of characters to be sent(start of array)
 * @param len number of chars to send
 * @param slave_addr address of slave
 * 
 * @return 0 = transmission success, -1 = transmission failed(slave didnt acknowledge)
 * 
 */
int i2c_master_write(uint8_t *buf, uint16_t len, uint8_t slave_addr){

    i2c->DR = (slave_addr << 1); //pad with 0 to indicate a write
    
    //need to wait for ack or nak
    while (!(i2c->SR1 & (SR1_ADDR | SR1_AF)));

    //if fail then send a stop bit and indicate a failure of reception
    if(i2c->SR1 & SR1_AF) return i2c_nack();

    (void)i2c->SR2; //have to read SR2 to clear ADDR bit in SR1
    
    for(uint16_t i = 0; i<len; i ++){
        //have to wait for data reg to clear or for a NAK shows up
        while (!(i2c->SR1 & (SR1_TxE | SR1_AF)));

        //if fail then send a stop bit and indicate a failure of reception
        if(i2c->SR1 & SR1_AF) return i2c_nack();

        i2c->DR = buf[i];        
    }
    //waits for BTF and TxE to be set or a NAK to come through
    while(!(((i2c->SR1 &SR1_TxE) && (i2c->SR1 & SR1_BTF)) || (i2c->SR1 & SR1_AF)));
    
    if(i2c->SR1 & SR1_AF) return i2c_nack();

    i2c_master_stop();

    return 0;
}

int i2c_master_read(uint8_t *buf, uint16_t len, uint8_t slave_addr){
    return 0;
}
