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

void i2c_master_start() {
    
}

void i2c_master_stop() {
    return;
}

int i2c_master_write(uint8_t *buf, uint16_t len, uint8_t slave_addr){
    (void) buf;
    (void) len;
    (void) slave_addr;

    return 0;
}

int i2c_master_read(uint8_t *buf, uint16_t len, uint8_t slave_addr){
    (void) buf;
    (void) len;
    (void) slave_addr;

    return 0;
}
