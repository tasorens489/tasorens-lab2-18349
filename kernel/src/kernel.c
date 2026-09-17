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
  
  //LEDR(A3 = PB_0)
  gpio_init(GPIO_B, 0, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
  //set LEDR high for now
  gpio_set(GPIO_B, 0);

  //LEDG(D13 = PA_5)
  gpio_init(GPIO_A, 5, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
  gpio_set(GPIO_A, 5);

  //Button 1(D6 = PB_10)
  gpio_init(GPIO_B, 10, MODE_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);
  //Button 2(D4 = PB_5)
  gpio_init(GPIO_B, 5, MODE_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);

  //counter so gpio_isn't contantly printing to terminal so often
  int counter = 0;

  uart_polling_init(USART_BRR);
  printk("hello world\n");

  //counter is an arbitrary number of cycles
  while(1){
    if(counter == 10000){
      int button1 = gpio_read(GPIO_B, 10);
      int button2 = gpio_read(GPIO_B, 5);
      printk("button 1 = %d   button 2 = %d", button1, button2);
      counter = 0;
    }
    counter++;
    //char c = uart_polling_get_byte();
    //uart_polling_put_byte(c);
  }
  return 0;
}
