#include <unistd.h>

#include <gpio.h>
#include <rcc.h>
#include <uart_polling.h>

/** @brief The UART register map. */
struct uart_reg_map {
    volatile uint32_t SR;   /**< Status Register */
    volatile uint32_t DR;   /**<  Data Register */
    volatile uint32_t BRR;  /**<  Baud Rate Register */
    volatile uint32_t CR1;  /**<  Control Register 1 */
    volatile uint32_t CR2;  /**<  Control Register 2 */
    volatile uint32_t CR3;  /**<  Control Register 3 */
    volatile uint32_t GTPR; /**<  Guard Time and Prescaler Register */
};

/** @brief Base address for UART2 */
#define UART2_BASE  (struct uart_reg_map *) 0x40004400

//Enable bit for UART2 in apb1_enr in RCC
#define UART_RCC_EN (1<<17)

//Sets UART enable, Transmit enable, Reciever enable
//Set in the CR1 register
#define UART_CTRL_EN (1<<13) | (1<<3) | (1<<2)

//bit mask for transmission register empty or not
#define UART_TXE (1<<7)

//bit mask for read register empty or not
#define UART_RXNE (1<<5)


struct uart_reg_map *uart = UART2_BASE;

/**
 * @brief initializes UART to given baud rate with 8-bit word length, 1 stop bit, 0 parity bits
 *
 * @param baud Baud rate
 */
void uart_polling_init (int baud){
    //UART TX(PA_2)
    gpio_init(GPIO_A, 2, MODE_ALT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);

    //UART RX(PA_3)
    gpio_init(GPIO_A, 3, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);

    struct rcc_reg_map *rcc = RCC_BASE;

    //enables UART CLOCK in the RCC
    rcc->apb1_enr |= UART_RCC_EN;

    //sets baud rate
    uart->BRR =  baud;

    //Enables UART and the transmitter and reciever
    uart->CR1 |= UART_CTRL_EN;
}

/**
 * @brief transmits a byte over UART
 *
 * @param c character to be sent
 */
void uart_polling_put_byte (char c){
    //stall while the transmit register is full
    while(!(uart->SR & UART_TXE));
    uart->DR = c;
}

/**
 * @brief receives a byte over UART
 */
char uart_polling_get_byte () {
    //wait while read reg is empty
    while(!(uart->SR & UART_RXNE));

    //need to cast register value to char
    return (char)uart->DR;
}
