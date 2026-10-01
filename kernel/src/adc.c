/**
 * @file adc.c
 *
 * @brief Polling driver for ADC1 on the STM32F401. Configures a 10-bit
 *        ADC and performs single conversions on a requested channel.
 *
 * @date 10/01/2026
 *
 * @author Thomas Sorensen(tasorens) & Marina Wang(marinaw)
 */

#include <gpio.h>
#include <stdint.h>
#include <rcc.h>
#include <adc.h>

struct adc_reg_map {
    volatile uint32_t SR;    // 0x00 
    volatile uint32_t CR1;   // 0x04 
    volatile uint32_t CR2;   // 0x08 
    volatile uint32_t SMPR1; // 0x0C 
    volatile uint32_t SMPR2; // 0x10
	volatile uint32_t JOFR1; // 0x14
	volatile uint32_t JOFR2; // 0x18
	volatile uint32_t JOFR3; // 0x1C
	volatile uint32_t JOFR4; // 0x20
	volatile uint32_t HTR;   // 0x24
	volatile uint32_t LTR;   // 0x28
	volatile uint32_t SQR1;  // 0x2C
	volatile uint32_t SQR2;  // 0x30
	volatile uint32_t SQR3;  // 0x34
	volatile uint32_t JSQR;  // 0x38
	volatile uint32_t JDR1;  // 0x3C
	volatile uint32_t JDR2;  // 0x40
	volatile uint32_t JDR3;  // 0x44
	volatile uint32_t JDR4;  // 0x48
    volatile uint32_t DR;    /* 0x4C */
};

#define ADC1_BASE (struct adc_reg_map *) 0x40012000

#define ADC1_EN_RCC (1 << 8)
#define CR1_RES_10B (1 << 24)
#define CR2_ADON (1 << 0)
#define CR2_SWSTART (1 <<30)
#define SR_EOC (1 <<1)
#define SQR3_SQ1_MASK 0x1F
#define DR_MASK 0xFFFF //adc is only 10 bits
#define ADON_SETTLE_CYC 100

static struct adc_reg_map *adc = ADC1_BASE;

/**
 * @brief initializes ADC1 for 10-bit single conversions on PA0 (A0)
 */
void adc_init() {
	//A0 - PA_0
	gpio_init(GPIO_A, 0, MODE_ANALOG_INPUT, OUTPUT_PUSH_PULL, 
				OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
	
	struct rcc_reg_map *rcc = (struct rcc_reg_map *)RCC_BASE;
	rcc->apb2_enr |= ADC1_EN_RCC; //tunrs on ADC clock

	adc->CR1 |= CR1_RES_10B; //sets 10 bit resolution
	adc->CR2 |= CR2_ADON; //turns the adc on
	
	//short wait loop for the ADC to settle
	for(volatile int i = 0; i<ADON_SETTLE_CYC; i++);
}

/**
 * @brief does a single blocking conversion on an ADC channel
 *
 * @param chan ADC channel number to read (0-18)
 * @return The 10-bit conversion result
 */
uint16_t adc_read_chan(uint8_t chan){
    adc->SQR3 &= ~SQR3_SQ1_MASK; //clears SQ1
	adc->SQR3 |= (chan & SQR3_SQ1_MASK);
	adc->CR2 |= CR2_SWSTART; //starts the conversion

	//just spins waiting for EOC to be set
	while(!(adc->SR & SR_EOC)){};

	return (uint16_t)(adc->DR & DR_MASK);
}
