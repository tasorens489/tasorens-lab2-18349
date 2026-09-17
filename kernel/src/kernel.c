#include <gpio.h>
#include <i2c.h>
#include <printk.h>
#include <uart_polling.h>
#include <unistd.h>
#include <lcd_driver.h>
#include <keypad_driver.h>
 
//calcialted for a baud rate of 115200 with OVER8 = 0
//Gives USARTDIV = 8.6806. 
#define USART_DIV_MANTISSA 8
#define USART_DIV_FRACTION 11
#define USART_BRR ((USART_DIV_MANTISSA << 4) | USART_DIV_FRACTION)


int kernel_main() {
  //baud is set using a define so parameter doesn't matter
  uart_polling_init(USART_BRR);
  printk("hello world\n");
  while(1){
    char c = uart_polling_get_byte();
    uart_polling_put_byte(c);
  }
  return 0;
}
